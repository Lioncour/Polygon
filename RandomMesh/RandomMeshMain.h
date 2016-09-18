#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\BackhroundRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace RandomMesh
{
	class RandomMeshMain
		: public DX::IDeviceNotify
	{
	public:
		RandomMeshMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~RandomMeshMain();
		void CreateWindowSizeDependentResources();
		void StartTracking(float x, float y) { m_pointerLocationX = x; m_pointerLocationY = y; m_sceneRenderer->StartTracking(x, y); }
		void TrackingUpdate(float x, float y) { m_pointerLocationX = x; m_pointerLocationY = y; }
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<BackhroundRenderer> m_backgroundRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
		float m_pointerLocationY;
	};
}