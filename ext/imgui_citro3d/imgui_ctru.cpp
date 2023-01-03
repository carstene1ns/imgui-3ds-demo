// copied from ftpd
//
// The MIT License (MIT)
//
// Copyright (C) 2020 Michael Theall
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "imgui_ctru.h"

#include "imgui.h"

#include "../imgui/imgui_internal.h"

#include <chrono>
#include <cstring>
#include <functional>
#include <string>
#include <tuple>
#include <cstdint>
#include <chrono>
using namespace std::chrono_literals;

namespace
{
/// \brief Clipboard
std::string s_clipboard;

/// \brief Get clipboard text callback
/// \param userData_ User data
char const *getClipboardText (void *const userData_)
{
	(void)userData_;
	return s_clipboard.c_str ();
}

/// \brief Set clipboard text callback
/// \param userData_ User data
/// \param text_ Clipboard text
void setClipboardText (void *const userData_, char const *const text_)
{
	(void)userData_;
	s_clipboard = text_;
}

/// \brief Update touch position
/// \param io_ ImGui IO
void updateTouch (ImGuiIO &io_)
{
	// check if touchpad was released
	if (hidKeysUp () & KEY_TOUCH)
	{
		// keep mouse position for one frame for release event
		io_.MouseDown[0] = false;
		return;
	}

	// check if touchpad is touched
	if (!(hidKeysHeld () & KEY_TOUCH))
	{
		// set mouse cursor off-screen
		io_.MousePos     = ImVec2 (-10.0f, -10.0f);
		io_.MouseDown[0] = false;
		return;
	}

	// read touch position
	touchPosition pos;
	hidTouchRead (&pos);

	// transform to bottom-screen space
	io_.MousePos     = ImVec2 (pos.px + 40.0f, pos.py + 240.0f);
	io_.MouseDown[0] = true;
}

/// \brief Update gamepad inputs
/// \param io_ ImGui IO
void updateGamepads (ImGuiIO &io_)
{
	// clear navigation inputs
	std::memset (io_.NavInputs, 0, sizeof (io_.NavInputs));

	auto const buttonMapping = {
	    std::make_pair (KEY_A, ImGuiNavInput_Activate),
	    std::make_pair (KEY_B, ImGuiNavInput_Cancel),
	    std::make_pair (KEY_X, ImGuiNavInput_Input),
	    std::make_pair (KEY_Y, ImGuiNavInput_Menu),
	    std::make_pair (KEY_L, ImGuiNavInput_FocusPrev),
	    std::make_pair (KEY_L, ImGuiNavInput_TweakSlow),
	    std::make_pair (KEY_ZL, ImGuiNavInput_FocusPrev),
	    std::make_pair (KEY_ZL, ImGuiNavInput_TweakSlow),
	    std::make_pair (KEY_R, ImGuiNavInput_FocusNext),
	    std::make_pair (KEY_R, ImGuiNavInput_TweakFast),
	    std::make_pair (KEY_ZR, ImGuiNavInput_FocusNext),
	    std::make_pair (KEY_ZR, ImGuiNavInput_TweakFast),
	    std::make_pair (KEY_DUP, ImGuiNavInput_DpadUp),
	    std::make_pair (KEY_DRIGHT, ImGuiNavInput_DpadRight),
	    std::make_pair (KEY_DDOWN, ImGuiNavInput_DpadDown),
	    std::make_pair (KEY_DLEFT, ImGuiNavInput_DpadLeft),
	};

	// read buttons from 3DS
	auto const keys = hidKeysHeld ();
	for (auto const &[in, out] : buttonMapping)
	{
		if (keys & in)
			io_.NavInputs[out] = 1.0f;
	}

	// update joystick
	circlePosition cpad;
	auto const analogMapping = {
	    std::make_tuple (std::ref (cpad.dx), ImGuiNavInput_LStickLeft, -0.3f, -0.9f),
	    std::make_tuple (std::ref (cpad.dx), ImGuiNavInput_LStickRight, +0.3f, +0.9f),
	    std::make_tuple (std::ref (cpad.dy), ImGuiNavInput_LStickUp, +0.3f, +0.9f),
	    std::make_tuple (std::ref (cpad.dy), ImGuiNavInput_LStickDown, -0.3f, -0.9f),
	};

	// read left joystick from circle pad
	hidCircleRead (&cpad);
	for (auto const &[in, out, min, max] : analogMapping)
	{
		auto const value   = in / static_cast<float> (0x9C);
		io_.NavInputs[out] = std::clamp ((value - min) / (max - min), 0.0f, 1.0f);
	}
}

/// \brief Update keyboard inputs
/// \param io_ ImGui IO
void updateKeyboard (ImGuiIO &io_)
{
	static enum {
		INACTIVE,
		KEYBOARD,
		CLEARED,
	} state = INACTIVE;

	switch (state)
	{
	case INACTIVE:
	{
		if (!io_.WantTextInput)
			return;

		auto &textState = ImGui::GetCurrentContext ()->InputTextState;

		SwkbdState kbd;

		swkbdInit (&kbd, SWKBD_TYPE_NORMAL, 2, -1);
		swkbdSetButton (&kbd, SWKBD_BUTTON_LEFT, "Cancel", false);
		swkbdSetButton (&kbd, SWKBD_BUTTON_RIGHT, "OK", true);
		swkbdSetInitialText (
		    &kbd, std::string (textState.InitialTextA.Data, textState.InitialTextA.Size).c_str ());

		if (textState.Flags & ImGuiInputTextFlags_Password)
			swkbdSetPasswordMode (&kbd, SWKBD_PASSWORD_HIDE_DELAY);

		char buffer[32]   = {0};
		auto const button = swkbdInputText (&kbd, buffer, sizeof (buffer));
		if (button == SWKBD_BUTTON_RIGHT)
			io_.AddInputCharactersUTF8 (buffer);

		state = KEYBOARD;
		break;
	}

	case KEYBOARD:
		// need to skip a frame for active id to really be cleared
		ImGui::ClearActiveID ();
		state = CLEARED;
		break;

	case CLEARED:
		state = INACTIVE;
		break;
	}
}
}

struct n3ds_clock
{
	/// \brief Type representing number of ticks
	using rep = uint64_t;

	/// \brief Type representing ratio of clock period in seconds
	using period = std::ratio<1, SYSCLOCK_ARM11>;

	/// \brief Duration type
	using duration = std::chrono::duration<rep, period>;

	/// \brief Timestamp type
	using time_point = std::chrono::time_point<n3ds_clock>;

	/// \brief Whether clock is steady
	constexpr static bool is_steady = true;

	/// \brief Current timestamp
	static time_point now () noexcept;
};

inline n3ds_clock::time_point n3ds_clock::now () noexcept
{
	return time_point (duration (svcGetSystemTick ()));
}


bool imgui::ctru::init ()
{
	auto &io = ImGui::GetIO ();

	// setup config flags
	io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// setup platform backend
	io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	io.BackendPlatformName = "3DS";

	// disable mouse cursor
	io.MouseDrawCursor = false;

	// clipboard callbacks
	io.SetClipboardTextFn = setClipboardText;
	io.GetClipboardTextFn = getClipboardText;
	io.ClipboardUserData  = nullptr;

	return true;
}

void imgui::ctru::newFrame ()
{
	auto &io = ImGui::GetIO ();

	// check that font was built
	IM_ASSERT (io.Fonts->IsBuilt () &&
	           "Font atlas not built! It is generally built by the renderer back-end. Missing call "
	           "to renderer _NewFrame() function?");

	// time step
	static auto const start = n3ds_clock::now ();
	static auto prev        = start;
	auto const now          = n3ds_clock::now ();

	io.DeltaTime = std::chrono::duration<float> (now - prev).count ();
	prev         = now;

	updateTouch (io);
	updateGamepads (io);
	updateKeyboard (io);
}
