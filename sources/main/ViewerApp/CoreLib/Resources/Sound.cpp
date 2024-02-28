//
// Created by pierr on 20/10/2023.
//

#include "Sound.h"

#include "ViewerApp/CoreLib/Plateform/GLFW/GlfwSound.h"

namespace Astra::UI::Core {
    Scope<Sound> Sound::CreateSound(int frequency) {
        return CreateScope<Glfw::GlfwSound>(frequency);
    }
}
