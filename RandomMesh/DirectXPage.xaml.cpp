﻿//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include <ppltasks.h>
#include "StlWriter.h"

using namespace RandomMesh;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	if (Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.UI.ViewManagement.StatusBar"))
	{
		auto statusbar = Windows::UI::ViewManagement::StatusBar::GetForCurrentView();
		statusbar->HideAsync();
	}

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// Register for pointer events, which will be raised on the background thread.
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);
		m_coreInput->PointerExited += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerExited);
		m_coreInput->PointerWheelChanged += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerWheelChanged);

		//Create a gesture recognizer to handle manipulation events and configure it for translation and rotation
		m_gestureRecognizer = ref new Windows::UI::Input::GestureRecognizer();

		m_gestureRecognizer->GestureSettings =
			Windows::UI::Input::GestureSettings::ManipulationTranslateX
			| Windows::UI::Input::GestureSettings::ManipulationTranslateY
			| Windows::UI::Input::GestureSettings::ManipulationRotate
			| Windows::UI::Input::GestureSettings::ManipulationScale;

		m_gestureRecognizer->ManipulationStarted +=
			ref new TypedEventHandler<GestureRecognizer^, ManipulationStartedEventArgs^>(this, &DirectXPage::OnManipulationStarted);
		m_gestureRecognizer->ManipulationUpdated +=
			ref new TypedEventHandler<GestureRecognizer^, ManipulationUpdatedEventArgs^>(this, &DirectXPage::OnManipulationUpdated);
		m_gestureRecognizer->ManipulationCompleted +=
			ref new TypedEventHandler<GestureRecognizer^, ManipulationCompletedEventArgs^>(this, &DirectXPage::OnManipulationCompleted);

		// Begin processing input messages as they're delivered.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Run task on a dedicated high priority background thread.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<RandomMeshMain>(new RandomMeshMain(m_deviceResources));
	m_main->StartRenderLoop();

	auto bridge = new EventsBridge(this);
	m_main->RegisterEventsHandler(bridge);

	UpdateBusyPanel();
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	m_main->RegisterEventsHandler(nullptr);
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Stop rendering when the app is suspended.
	m_main->StopRenderLoop();

	// Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	m_main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	m_main->StartTracking(0.0, 0.0, 0.0, 1.0);
	m_gestureRecognizer->ProcessDownEvent(e->CurrentPoint);
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	IVector<PointerPoint^>^ pointerPoints = PointerPoint::GetIntermediatePoints(e->CurrentPoint->PointerId);
	m_gestureRecognizer->ProcessMoveEvents(pointerPoints);
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	m_gestureRecognizer->ProcessUpEvent(e->CurrentPoint);
	m_main->StopTracking();
}

void DirectXPage::OnPointerExited(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ args)
{
	m_gestureRecognizer->ProcessUpEvent(args->CurrentPoint);
	m_main->StopTracking();
}

void DirectXPage::OnPointerWheelChanged(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ args)
{
	m_gestureRecognizer->ProcessMouseWheelEvent(args->CurrentPoint, false, true);
}

static void Log(const wchar_t *text, ManipulationDelta delta)
{
#if _DEBUG
	wchar_t buf[1024];
	_snwprintf_s(buf, 1024, _TRUNCATE, L"%s Tran:(%f, %f), Exp:%f, Rot:%f, Scale:%f\r\n", text, delta.Translation.X, delta.Translation.Y, delta.Expansion, delta.Rotation, delta.Scale);
	OutputDebugString(buf);
#endif
}

void DirectXPage::OnManipulationStarted(Windows::UI::Input::GestureRecognizer ^ sender, Windows::UI::Input::ManipulationStartedEventArgs ^ args)
{	
	auto delta = args->Cumulative;
	Log(L"MS", delta);

	m_main->StartTracking(delta.Translation.X, delta.Translation.Y, delta.Rotation, delta.Scale);
}

void DirectXPage::OnManipulationUpdated(Windows::UI::Input::GestureRecognizer ^ sender, Windows::UI::Input::ManipulationUpdatedEventArgs ^ args)
{
	auto delta = args->Cumulative;	
	Log(L"MU", delta);

	m_main->TrackingUpdate(delta.Translation.X, delta.Translation.Y, delta.Rotation, delta.Scale);
}

void DirectXPage::OnManipulationCompleted(Windows::UI::Input::GestureRecognizer ^ sender, Windows::UI::Input::ManipulationCompletedEventArgs ^ args)
{
	auto cum = args->Cumulative;
	Log(L"MC", cum);

	m_main->StopTracking();
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}

void RandomMesh::DirectXPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->GenerateNewMesh();
}

void RandomMesh::DirectXPage::UpdateBusyPanel()
{
	Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
	{
		viewBusyPanel->Visibility = m_main->GetIsGenerating()
			? Windows::UI::Xaml::Visibility::Visible
			: Windows::UI::Xaml::Visibility::Collapsed;
	}));
}


void RandomMesh::DirectXPage::SaveAsStlClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	using namespace Windows::Storage;
	using namespace Concurrency;

	auto savePicker = ref new Pickers::FileSavePicker();
	savePicker->SuggestedStartLocation = Pickers::PickerLocationId::DocumentsLibrary;
	savePicker->SuggestedFileName = "My figure";

	savePicker->FileTypeChoices->Insert("STL", ref new Platform::Collections::Vector<Platform::String^>(1, ".stl"));

	auto task = create_task(savePicker->PickSaveFileAsync()).then([this](StorageFile^ file)
	{
		return file->OpenAsync(FileAccessMode::ReadWrite);
	}).then([this](Streams::IRandomAccessStream^ stream)
	{
		stream->Size = 0;

		auto writer = ref new Streams::DataWriter(stream->GetOutputStreamAt(0));
		RandomMesh::StlWriter::Save(writer, m_main->GetMesh());

		writer->StoreAsync();
	}).then([](Concurrency::task<void> t)
	{
		try
		{
			t.get();
			OutputDebugString(L"Stl saved.");
		}
		catch (...)
		{
			OutputDebugString(L"Error on STL save");
		}
	});;
}


void RandomMesh::DirectXPage::WhiteBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->SetBackgroundColor(1.0f, 1.0f, 1.0f, 1.0f);
	TogglePaneButton->IsChecked = false;
}

void RandomMesh::DirectXPage::GrayBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->SetBackgroundColor(0.75f, 0.75f, 0.75f, 0.75f);
	TogglePaneButton->IsChecked = false;
}

void RandomMesh::DirectXPage::BlackBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->SetBackgroundColor(0.0f, 0.0f, 0.0f, 1.0f);
	TogglePaneButton->IsChecked = false;
}

void RandomMesh::DirectXPage::ImageBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->ResetBackground();
	TogglePaneButton->IsChecked = false;
}
