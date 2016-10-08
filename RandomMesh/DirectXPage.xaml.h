//
// DirectXPage.xaml.h
// Declaration of the DirectXPage class.
//

#pragma once

#include "DirectXPage.g.h"

#include "Common\DeviceResources.h"
#include "RandomMeshMain.h"

namespace RandomMesh
{
	/// <summary>
	/// A page that hosts a DirectX SwapChainPanel.
	/// </summary>
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		virtual ~DirectXPage();

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

		void UpdateBusyPanel();

	private:
		// XAML low-level rendering event handler.
		void OnRendering(Platform::Object^ sender, Platform::Object^ args);

		// Window event handlers.
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

		// DisplayInformation event handlers.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// Other event handlers.
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		// Track our independent input on a background worker thread.
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;
		Windows::UI::Input::GestureRecognizer^ m_gestureRecognizer;

		// Independent input handling functions.
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerExited(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerWheelChanged(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);

		// Gesture recognizer event handlers
		void OnManipulationStarted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationStartedEventArgs^ args);
		void OnManipulationUpdated(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationUpdatedEventArgs^ args);
		//void OnManipulationInertiaStarting(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationInertiaStartingEventArgs^ args);
		void OnManipulationCompleted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationCompletedEventArgs^ args);

		// Resources used to render the DirectX content in the XAML page background.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<RandomMeshMain> m_main;
		bool m_windowVisible;
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveAsStlClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void WhiteBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void GrayBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void BlackBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ImageBackgroundClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void HyperlinkButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	private struct EventsBridge
		: IRandomMeshEvents
	{
	private:
		DirectXPage^ m_page;

	public:
		EventsBridge(DirectXPage^ page)
		{
			m_page = page;
		}

		virtual void MeshGenerating(bool isGenerating)
		{
			if (m_page == nullptr)
			{
				return;
			}

			m_page->UpdateBusyPanel();
		}
	};
}

