// Cpp STL

// Project
#include "DGL.hpp"
#include "Actions.hpp"
#include "Testing.hpp"

#include "Cpp_Alias.hpp"



namespace Execution
{
	inline namespace Alias
	{
		// DGL

		using DGL::Camera       ;
		using DGL::ECursorMode  ; 
		using DGL::EDirection   ;
		using DGL::EFrameBuffer ;
		using DGL::EKeyCodes    ;
		using DGL::EMouseMode   ; 
		using DGL::EPrimitives  ;
		using DGL::ERotationAxis;
		using DGL::gFloat       ;
		using DGL::LinearColor  ;
		using DGL::Matrix       ;
		using DGL::Matrix4x4    ;
		using DGL::TimeValDec   ;
		using DGL::Vector3      ;
		using DGL::Window       ;

		using DGL::SimpleShader;

		using DGL::CanUseRawMouse             ;
		using DGL::ClearBuffer                ;
		using DGL::CreateWindow               ;
		using DGL::BindVertexArray            ;
		using DGL::DisableVertexAttributeArray;
		using DGL::EnableVertexAttributeArray ;
		using DGL::GetCursorPosition          ;
		using DGL::GetTime                    ;
		using DGL::InitalizeGLEW              ;
		using DGL::InitalizeGLFW              ;
		using DGL::KeyPressed                 ;	
		using DGL::NotShared                  ;
		using DGL::PollEvents                 ;
		using DGL::ResetCursor                ;
		using DGL::SetClearColor              ;
		using DGL::SetCurrentContext          ;
		using DGL::SetInputMode               ;
		using DGL::SetPolygonMode             ;
		using DGL::SetUniformVariable_MVA     ;
		using DGL::SwapBuffers                ;
		using DGL::UseProgramShader           ;
		using DGL::TerminateGLFW              ;
		using DGL::WindowedMode               ;

		using DGL::DefaultSpace::ScreenWidth       ;
		using DGL::DefaultSpace::ScreenHeight      ;
		using DGL::DefaultSpace::ScreenCenterHeight;
		using DGL::DefaultSpace::ScreenCenterWidth ; 
		using DGL::DefaultSpace::Screenspace       ;
		using DGL::DefaultSpace::WorldCamera       ;

		using DGL::DefaultSpace::UpdateScreenspace;

		// Actions

		using Actions::ActionQueue;
	}


	// Globals

	bool Exist = true;   // Determines if the the execution should exit cycler.

	TimeValDec CycleStart                     ,    // Snapshot of cycle loop start time. 
		       CycleEnd                       ,    // Snapshot of cycle loop end   time. 
		       DeltaTime                      ,    // Delta between last cycle start and end. 
		       InputInterval   = 1.0f / 240.0f,    // Interval per second to complete the input   process of the cycle.
		       PhysicsInterval = 1.0f / 120.0f,    // Interval per second to complete the physics process of hte cycle. 
		       RenderInterval  = 1.0f /  60.0f ;   // Interval per second to complete the render  process of the cycle.

	ptr<Window> DefaultWindow;   // Default window to use for execution.

	double CursorX, CursorY;   // Cursor axis position on the window.

	gFloat CamMoveSpeed     = 8.0f,    // Rate at which the camera should move.
		   CamRotationSpeed = 5.0f ;   // Rate at which the camera should rotate.

	TimeValDec InputDelta   = 0.0,    // Current delta since last input   process. 
		       PhysicsDelta = 0.0,    // Current delta since last physics process. 
		       RenderDelta  = 0.0 ;   // Current delta since last render  process.

	ActionQueue ActionsToComplete;   // Actions queue to run during the physics process of the cycle.



	// Functionality

	// Temp fix for now... not sure how to make proper action handling that can reference member function delegates...

	sfn RotateCamera(ERotationAxis _rotationAxis, gFloat _rotationAmount, gFloat _delta)
	{
		WorldCamera.Rotate(_rotationAxis, _rotationAmount, _delta);
	} 
	
	deduce RotateCamDelegate = Delegate<decltype(RotateCamera)>(RotateCamera);

	sfn MoveCamera(EDirection _direction, gFloat _translationAmount, gFloat _delta)
	{
		WorldCamera.Move(_direction, _translationAmount, _delta);
	} 
	
	deduce MoveCamDelegate = Delegate<decltype(MoveCamera)>(MoveCamera);

	// End of temp stuff...



	// Currently Does everything required before entering the cycler.
	sfn PrepWorkspace()
	{
		InitalizeGLFW();

		DefaultWindow = CreateWindow(ScreenWidth, ScreenHeight, "Assignment 1: Lighting Test", WindowedMode(), NotShared());

		SetCurrentContext(DefaultWindow);

		InitalizeGLEW();   // Glew must initialize only after a context is set.

		// Cursor stuff

		SetInputMode(DefaultWindow, DGL::EMouseMode::Cursor, DGL::ECursorMode::Disable);

		ResetCursor(DefaultWindow, ScreenCenterWidth, ScreenCenterHeight);

		if (CanUseRawMouse())
		{
			SetInputMode(DefaultWindow, DGL::EMouseMode::RawMouse, DGL::EBool::True);
		}

		// End of cursor stuff...

		PrepareRenderObjects();

		SetPolygonMode(DGL::EFace::Front_and_Back, DGL::ERenderMode::Fill);
	}



	/*
	Cycles the process of what to do while a window is open.

	The input, physics, and render procedures can be specified with extra functionality by specifying delegates to those procedures.

	Cycler is hardcoded to exit if escape key is pressed.
	*/
	sfn Cycler(Delegate< Func<void, ptr<Window>> > _inputProcedure, Delegate< Func<void>> _physicsProcedure, Delegate< Func<void>> _renderProcedure)
	{
		while (Exist)
		{
			CycleStart = GetTime();

			if (InputDelta >= InputInterval)
			{
				PollEvents();

				if (KeyPressed(DefaultWindow, EKeyCodes::Escape))
				{
					Exist = false;
				}

				GetCursorPosition(DefaultWindow, Address(CursorX), Address(CursorY));

				_inputProcedure(DefaultWindow);

				ResetCursor(DefaultWindow, ScreenCenterWidth, ScreenCenterHeight);

				InputDelta = 0.0;
			}

			if (PhysicsDelta >= PhysicsInterval)
			{
				while (ActionsToComplete.HasAction())
				{
					ActionsToComplete.DoNextAction();
				}

				_physicsProcedure();

				PhysicsDelta = 0.0;
			}

			if (RenderDelta >= RenderInterval)
			{
				ClearBuffer(EFrameBuffer::Color, EFrameBuffer::Depth);

				SetClearColor(LinearColor(0.12f, 0.12f, 0.12f, 1.0f));

				_renderProcedure();

				SwapBuffers(DefaultWindow);

				RenderDelta = 0.0;
			}

			CycleEnd = GetTime();

			DeltaTime = CycleEnd - CycleStart;

			InputDelta   += DeltaTime;
			PhysicsDelta += DeltaTime;
			RenderDelta  += DeltaTime;
		}

		return;
	}



	sfn InputProcedure(ptr<Window> _currentWindowContext)
	{
		if (KeysPressed(_currentWindowContext, EKeyCodes::LeftShift, EKeyCodes::F1))
		{
			ECursorMode cursorMode = ECursorMode(GetMouseInputMode(DefaultWindow, EMouseMode::Cursor));

			deduce delegate = Delegate<decltype(SetInputMode<ECursorMode>)>(SetInputMode<ECursorMode>);

			if (cursorMode == ECursorMode::Normal || cursorMode == ECursorMode::Hidden)
			{
				ActionsToComplete.AddToQueue(delegate, _currentWindowContext, EMouseMode::Cursor, ECursorMode::Disable);
			}
			else
			{
				ActionsToComplete.AddToQueue(delegate, _currentWindowContext, EMouseMode::Cursor, ECursorMode::Normal);
			}
		}

		if (CursorX != 0)
		{
			ActionsToComplete.AddToQueue(RotateCamDelegate, ERotationAxis::Yaw, CursorX * CamMoveSpeed, PhysicsDelta);
		}

		if (CursorY != 0)
		{
			ActionsToComplete.AddToQueue(RotateCamDelegate, ERotationAxis::Pitch, CursorY * CamMoveSpeed, PhysicsDelta);
		}
		
		if (KeyPressed(_currentWindowContext, EKeyCodes::E))
		{
			ActionsToComplete.AddToQueue(MoveCamDelegate, EDirection::Up, CamMoveSpeed, PhysicsDelta);
		}

		if (KeyPressed(_currentWindowContext, EKeyCodes::Q))
		{
			ActionsToComplete.AddToQueue(MoveCamDelegate, EDirection::Down, CamMoveSpeed, PhysicsDelta);
		}

		if (KeyPressed(_currentWindowContext, EKeyCodes::A))
		{
			ActionsToComplete.AddToQueue(MoveCamDelegate, EDirection::Left, CamMoveSpeed, PhysicsDelta);
		}

		if (KeyPressed(_currentWindowContext, EKeyCodes::D))
		{
			ActionsToComplete.AddToQueue(MoveCamDelegate, EDirection::Right, CamMoveSpeed, PhysicsDelta);
		}

		if (KeyPressed(_currentWindowContext, EKeyCodes::W))
		{
			ActionsToComplete.AddToQueue(MoveCamDelegate, EDirection::Forward, CamMoveSpeed, PhysicsDelta);
		}

		if (KeyPressed(_currentWindowContext, EKeyCodes::S))
		{
			ActionsToComplete.AddToQueue(MoveCamDelegate, EDirection::Backward, CamMoveSpeed, PhysicsDelta);
		}
	}



	sfn PhysicsProcedure()
	{
		WorldCamera.UpdateCamera();

		UpdateScreenspace();

		DGL::SS_Transformed::UpdateShader(Screenspace);
	}

	

	sfn RenderProcedure() -> void
	{
		EnableVertexAttributeArray(VertexAttributeIndex);

		UseProgramShader(DGL::SS_Transformed::Shader);

		BindVertexArray(VertexArrayObj);

		DrawElements(EPrimitives::Triangles, 6, EDataType::UnsignedInt, ZeroOffset());

		DisableVertexAttributeArray(VertexAttributeIndex);
	}
	


	// Runtime Execution: Default Execution

	sfn Execute() -> ExitCode
	{
		PrepWorkspace();

		Cycler(InputProcedure, PhysicsProcedure, RenderProcedure);

		TerminateGLFW();

		return ExitCode::Success;
	}
}



int main(void)
{
	return int(Execution::Execute());
}

