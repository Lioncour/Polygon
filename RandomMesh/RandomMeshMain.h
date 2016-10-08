#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\MeshRenderer.h"
#include "Content\BackhroundRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace RandomMesh
{
	interface IRandomMeshEvents
	{
		virtual void MeshGenerating(bool isGenerating) = 0;
	};

	class RandomMeshMain
		: public DX::IDeviceNotify
	{
	public:
		RandomMeshMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~RandomMeshMain();
		void CreateWindowSizeDependentResources();

		void StartTracking(float x, float y, float zAngle, float scale)
		{
			m_pointerLocationX = x; 
			m_pointerLocationY = y; 
			m_zAngle = zAngle,
			m_scale = scale;

			m_sceneRenderer->StartTracking(x, y, m_zAngle, m_scale);
		}

		void TrackingUpdate(float x, float y, float zAngle, float scale)
		{
			m_pointerLocationX = x;
			m_pointerLocationY = y;
			m_zAngle = zAngle,
			m_scale = scale;

			Concurrency::critical_section::scoped_lock lock(m_criticalSection);
			m_sceneRenderer->TrackingUpdate(m_pointerLocationX, m_pointerLocationY, m_zAngle, m_scale);
		}
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }

		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		void GenerateNewMesh();
		bool GetIsGenerating() { return m_isGenerating; }
		void RegisterEventsHandler(IRandomMeshEvents* handler) { m_eventsHandler = handler; }

		shared_ptr<Mesh> GetMesh() { return m_mesh; }

		void SetBackgroundColor(float r, float g, float b, float a);
		void ResetBackground();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		void SetIsGenerating(bool isGenerating);

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<MeshRenderer> m_sceneRenderer;
		std::unique_ptr<BackhroundRenderer> m_backgroundRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
		float m_pointerLocationY;
		float m_zAngle;
		float m_scale;

		bool m_isGenerating;
		IRandomMeshEvents* m_eventsHandler;

		shared_ptr<Mesh> m_mesh;

		shared_ptr<XMVECTORF32> m_backgroundColor;
	};
}