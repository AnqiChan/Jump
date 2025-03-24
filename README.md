# Anqi's Jump

## Overview
This repository contains the code for an C Programming Language course project inspired by the popular WeChat Mini Program "跳一跳." The project aims to implement core gameplay mechanics.

## Development Status  ༼ つ ◕_◕ ༽つ
This is my first project and the code looks horrible. I'll be improving it and fixing bugs when I get more time. But if you spot anything odd and feel like helping out, your contributions are more than welcome!

## File Structure
```
shapez-course-project/
├── jump.c              # Source code
├── res/                # Image resources
├── CMakeLists.txt      
├── README.md           # Project documentation
```

## Requirements
- **Libraries**:
  - SDL2
  - SDL2_image
  - SDL2_ttf
  - SDL2_mixer

Build and run the game by running the following command in the terminal:
```bash
cd "path_to_game" ; if ($?) { gcc Jump.c -o Jump -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf } ; if ($?) { .\Jump }
```

## License
This project is for educational purposes and follows an open-source license (MIT License). See `LICENSE` for details.

## Acknowledgments
- Inspired by [Shapez](https://shapez.io/).
- Developed as part of a course project.
- Thanks to all testers and the CPL teaching assistants for their support and feedback.

