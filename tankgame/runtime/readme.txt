Tank! Game  readme.txt
for Windows98/2000/XP(OpenGL required)
Second Iteration
(C) Brien Smith-Martinez

Outsmart when outnumbered. 
3D Tank shooter, 'Tank! Game'.


- How to install.

Unzip and run TankGame.exe
Us settings.txt to change resolution and to toggle fullscreen mode


- How to play.

Mouse and Keyboard Controls:

 - Aim           Mouse
 - Shoot         Left Mouse Button
 - Special       Right Mouse Button
 - Forward       [W]
 - Backward      [S]
 - Rotate Left   [A]
 - Rotate Right  [D]
 - Jump          [SPACE]
 - Turbo         [RIGHT SHIFT]
 - Pause         [PAUSE]

 - Change camera [C]
 - Camera Height [UP/DOWN ARROWS]

Joystick:
If you have a joystick plugged in, you can use it to select the game mode.
If you do, the game will be controlled by the joystick.
If a 2 player mode is selected with the joystick, both players must use a joystick
If a 2 player mode is selected with the keyboard, one player will use the keyboard/mouse,
and the other player must use a joystick.

Different joysticks are handled differently.
As an example, here is how the PS2 controller is done:

 - Aim           Right Analog stick
 - Shoot         R2
 - Special       R1
 - Movement      Left Analog stick
 - Jump          L2
 - Turbo         L1       
 - Change Camera D-Pad

At the title screen, push enter or joystick button 1 to start the game.

The objective at each level is reach the exit to the next level.
The last enemy tank to die will become an exit.
On some levels, the exit can be found without killing enemies.

The game revolves around two energy bars:
The red one is your life (Love). If it runs out, you die and the level restarts
The blue one is your energy (Action). It is used for the following:
 - Shooting
 - Jumping
 - Turbo (Fast Jumping/Moving)

Your energy bars recharge. While the Action is recharging, Love will drain slightly.

Combo System: there are two bars on the right side of the screen. 
The red one is your combo meter. The blue one is your Special attack meter. The combo meter is raised by 
shooting enemy tanks repeatedly without missing and by destroying enemy tanks. When you destroy a tank,
the blue Special meter will rise to meet the red combo meter.
When the special meter is high enough, you can use a special attack. Different color tanks have different specials.

There are different colors of tanks.

Red: Rapid fire, low damage.
Blue: High damage, slow fire.
Yellow: Bouncy shot, moderate fire rate and low damage.
Purple: Curvy shot. Move the turret while shooting to curve your shot.
Grey: Low damage, slow fire, weak armor.

Tips:
Enemy tanks can harm eachother and will not avoid it.

Game Modes:
Solo-
Single player mode.

Co-Op-
2 Player split-screen. One player must use a joystick.

Versus-
The same as Co-Op, except the object is to kill the other player.

- Comments

If you have any comments, please mail to bmsgg6@umr.edu


- Webpage

Tank! Game webpage:
http://web.umr.edu/~bmsgg6/tank/

- Credits

Brien Smith-Martinez - programming, etc
James Lane Ramirez - music, SFX, model format programming
Alias Seiichi Tagami - player tank modeling


- Other Acknowledgement

Simple DirectMedia Layer is used for the display handling / input. 
 Simple DirectMedia Layer
 http://www.libsdl.org/

SDL_mixer and Ogg Vorbis CODEC to play BGM/SE. 
 SDL_mixer 1.2
 http://www.libsdl.org/projects/SDL_mixer/
 Vorbis.com
 http://www.vorbis.com/


- History

2004  Sept 20  First Iteration
            -Basic Game mechanics work

2005  March 28  Second Iteration in Progress
            -Game is better

2005  April 17  Second Iteration
			Game is better still, here's how:
            Game Mechanics:
			-A weapon system for variety
			-Joystick/Gamepad support
			-Combo/Special attack system
			-2 Player split-screen with Co-Op and Versus modes
			-Vastly improved tank to level collisions
			-Improved enemy AI
			-Decreased difficulty in early levels

			Details
			-Particle effects for more visual feedback
			-More sound effects for audio feedback
			-A little more variety in terrain texturing

			Technical Issues
			-Fixed mysterious crashes that occured on some WinXP machines
			-Config file to toggle Fullscreen mode and change resolution



-- License

License
-------

Copyright 2005 Brien Smith-Martinez. All rights reserved. 

Redistribution and use in source and binary forms, 
with or without modification, are permitted provided that 
the following conditions are met: 

 1. Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer. 

 2. Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
