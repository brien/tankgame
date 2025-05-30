#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#else
// If building on Linux:
#include <GL/gl.h>
#endif

#include "TankHandler.h"
#include "LevelHandler.h"
#include "GlobalTimer.h"
#include "math.h"
#include "App.h"
#include <string>

TankHandler::TankHandler()
    : numPlayers(1),
      numAttackingTanks(0),
      combo{0, 0},
      special{0, 0},
      comboNum{0, 0},
      hitCombo{0, 0},
      wins{0, 0},
      isInputJoy(false),
      difficultySetting(0)
{
}

TankHandler::~TankHandler()
{
    tanks.clear();
}

void TankHandler::Init()
{
    InitializePlayerTanks();
    InitializePlayerControls();
    InitializeEnemyTanks();
}

void TankHandler::InitializePlayerTanks()
{
    combo[0] = 0;
    combo[1] = 0;
    special[0] += (float)comboNum[0];
    special[1] += (float)comboNum[1];
    comboNum[0] = 0;
    comboNum[1] = 0;

    for (int j = 0; j < numPlayers; j++)
    {
        players[j].Init();
        players[j].isPlayer = true;

        if (App::GetSingleton().gameTask->versus)
        {
            players[j].x = LevelHandler::GetSingleton().enemy[8][0];
            players[j].z = LevelHandler::GetSingleton().enemy[8][1];

            players[0].x = LevelHandler::GetSingleton().enemy[9][0];
            players[0].z = LevelHandler::GetSingleton().enemy[9][1];
        }
        players[j].x += j;

        players[j].id = -1 * (j + 1);

        players[j].r = 1;
        players[j].g = 0;
        players[j].b = 0;

        players[j].r2 = 1;
        players[j].g2 = 1;
        players[j].b2 = 1;

        players[j].jid = j;

        players[j].inputMode = j == 0 ? InputMode::MODE_KEYBOARD_MOUSE : InputMode::MODE_JOYSTICK_GENERIC;

        players[j].SetType(players[j].type1, players[j].type2);

        if (difficultySetting == 0)
        {
            players[j].energyRegen *= 2;
            players[j].maxEnergy *= 3;
            players[j].energy *= 3;
        }
        else if (difficultySetting == 1)
        {
            players[j].energyRegen *= 1.5;
            players[j].maxEnergy *= 1.5;
            players[j].energy *= 1.5;
        }

        players[j].rotRate *= 2;
    }
}

void TankHandler::InitializePlayerControls()
{
    if (numPlayers > 1)
    {
        if (SDL_NumJoysticks() == 1)
        {
            players[1].jid = 0;
            if (InputTask::joynames[0].find("PS2") != string::npos || InputTask::joynames[0].find("MP-8866") != string::npos)
                players[1].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            else if (InputTask::joynames[0].find("Extreme 3D") != string::npos)
                players[1].inputMode = InputMode::MODE_EXTREME_3D;
            else if (InputTask::joynames[0].find("NGC") != string::npos || InputTask::joynames[0].find("GameCube") != string::npos)
                players[1].inputMode = InputMode::MODE_NINTENDO_GC;
        }
        if (SDL_NumJoysticks() > 1)
        {
            players[0].jid = 1;
            players[1].jid = 0;
            players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            players[1].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            if (InputTask::joynames[0].find("PlayStation") != string::npos || InputTask::joynames[0].find("Playstation") != string::npos || InputTask::joynames[1].find("PS2") != string::npos || InputTask::joynames[1].find("MP-8866") != string::npos)
                players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            else if (InputTask::joynames[1].find("Extreme 3D") != string::npos)
                players[0].inputMode = InputMode::MODE_EXTREME_3D;
            else if (InputTask::joynames[1].find("NGC") != string::npos || InputTask::joynames[1].find("GameCube") != string::npos)
                players[0].inputMode = InputMode::MODE_NINTENDO_GC;

            if (InputTask::joynames[0].find("PS2") != string::npos || InputTask::joynames[0].find("MP-8866") != string::npos)
                players[1].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            else if (InputTask::joynames[0].find("Extreme 3D") != string::npos)
                players[1].inputMode = InputMode::MODE_EXTREME_3D;
            else if (InputTask::joynames[0].find("NGC") != string::npos || InputTask::joynames[0].find("GameCube") != string::npos)
                players[1].inputMode = InputMode::MODE_NINTENDO_GC;
        }
    }
    else if (SDL_NumJoysticks() > 0 && isInputJoy)
    {
        players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
        if (InputTask::joynames[0].find("PlayStation") != string::npos || InputTask::joynames[0].find("Playstation") != string::npos || InputTask::joynames[0].find("PS2") != string::npos || InputTask::joynames[0].find("MP-8866") != string::npos)
            players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
        else if (InputTask::joynames[0].find("Extreme 3D") != string::npos)
            players[0].inputMode = InputMode::MODE_EXTREME_3D;
        else if (InputTask::joynames[0].find("NGC") != string::npos || InputTask::joynames[0].find("GameCube") != string::npos)
            players[0].inputMode = InputMode::MODE_NINTENDO_GC;
    }
}

void TankHandler::InitializeEnemyTanks()
{
    tanks.clear();

    int enemyCount = 5 + 3 * (LevelHandler::GetSingleton().levelNumber - 48);

    for (int i = 0; i < enemyCount; ++i)
    {
        Tank tempTank;

        tempTank.Init();

        if (i < 16)
        {
            tempTank.x = LevelHandler::GetSingleton().enemy[i][0];
            tempTank.z = LevelHandler::GetSingleton().enemy[i][1];
        }
        else
        {
            tempTank.x = LevelHandler::GetSingleton().enemy[i % 16][0] + i % 2;
            tempTank.z = LevelHandler::GetSingleton().enemy[i % 16][1] + i % 2;
        }

        tempTank.y = LevelHandler::GetSingleton().GetTerrainHeight((int)tempTank.x, (int)tempTank.z);
        tempTank.ry = i * 45;
        tempTank.id = i;

        if ((LevelHandler::GetSingleton().levelNumber - 48) == 0)
        {
            tempTank.SetType(static_cast<TankType>(1 + i % 3), TankType::TYPE_GREY);
            if (tempTank.type1 == TankType::TYPE_BLUE)
                tempTank.SetType(TankType::TYPE_YELLOW, TankType::TYPE_GREY);
            if (tempTank.id == 4)
            {
                tempTank.SetType(TankType::TYPE_BLUE, TankType::TYPE_GREY);
                tempTank.attack = tempTank.attack / 3;
            }
        }
        else
        {
            if ((LevelHandler::GetSingleton().levelNumber - 48) == 2 || (LevelHandler::GetSingleton().levelNumber - 48) == 5 || (LevelHandler::GetSingleton().levelNumber - 48) == 12)
            {
                tempTank.SetType(static_cast<TankType>(i % 4 + 1), static_cast<TankType>(i % 3 + 1));
            }
            else
            {
                tempTank.SetType(static_cast<TankType>(1 + i % 3), static_cast<TankType>(i % 3));
            }
        }
        tempTank.jumpCost = 0;

        tanks.push_back(tempTank);
    }
}

void TankHandler::NextFrame()
{
    for (int i = 0; i < numPlayers; i++)
    {
        if (combo[i] > 0.0f)
        {
            combo[i] -= 1.0f * GlobalTimer::dT;
            if (combo[i] < 0.0f)
            {
                combo[i] = 0.0f;
            }
        }
        else
        {
            comboNum[i] = 0;
        }

        if (players[i].alive)
        {
            players[i].NextFrame();
            players[i].HandleInput();
            players[i].deadtime = 0.0f;
        }
        else
        {
            players[i].Die();
            if (App::GetSingleton().gameTask->versus)
            {
                if (players[i].deadtime > 1.5)
                {
                    LevelHandler::GetSingleton().NextLevel(true);
                }
            }
            else
            {
                if (players[i].deadtime > 0.5)
                    LevelHandler::GetSingleton().NextLevel(false);
            }
            players[i].deadtime += GlobalTimer::dT;
        }
    }

    float dist;
    players[0].dist = 2024;
    players[1].dist = 2024;

    if (!tanks.empty())
    {
        auto update_player_target = [](Tank &player, const Tank &tank)
        {
            float dist = sqrt((tank.x - player.x) * (tank.x - player.x) + (tank.z - player.z) * (tank.z - player.z));
            if (dist < player.dist)
            {
                player.dist = dist;
                float ratio = (double)(player.z - tank.z) / (double)(player.x - tank.x);
                float rtyp = toDegrees(atan(ratio));
                if (tank.x < player.x)
                {
                    rtyp += 180;
                }
                player.rr = rtyp;
                player.rrl = player.rr;
            }
        };

        vector<Tank>::iterator it;
        for (it = tanks.begin(); it != tanks.end();)
        {
            if (it->alive)
            {
                it->AI();
                it->NextFrame();

                for (int p = 0; p < 2; ++p)
                {
                    update_player_target(players[p], *it);
                }

                ++it;
            }
            else
            {
                it->Die();
                it = tanks.erase(it);
            }
        }
    }

    if (App::GetSingleton().gameTask->versus)
    {
        players[0].dist = sqrt((players[0].x - players[1].x) * (players[0].x - players[1].x) + (players[0].z - players[1].z) * (players[0].z - players[1].z));
        players[1].dist = players[0].dist;

        float ratio = (double)(players[0].z - players[1].z) / (double)(players[0].x - players[1].x);

        float rtyp = toDegrees(atan(ratio));

        if (players[1].x < players[0].x)
        {
            rtyp += 180;
        }

        players[0].rr = rtyp;

        players[0].rrl = players[0].rr;

        rtyp += 180;

        players[1].rr = rtyp;

        players[1].rrl = players[1].rr;
    }

    LevelHandler::GetSingleton().ItemCollision();
}

void TankHandler::DrawTanks()
{
    static float drift = 0;

    for (int k = 0; k < numPlayers; ++k)
    {
        if (players[k].alive)
        {
            glPushMatrix();

            glDisable(GL_TEXTURE_2D);

            glEnable(GL_LIGHTING);

            // glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);

            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS); // This is the key

            // players[k].Draw();
            // glCallList(App::GetSingleton().graphicsTask ->bodylist);

            //-------------
            glPushMatrix();

            // glDisable(GL_TEXTURE_2D);
            // glDisable(GL_LIGHTING);

            glTranslatef(players[k].x, players[k].y + .05, players[k].z);
            glRotatef(players[k].rx, 1, 0, 0);
            glRotatef(-players[k].ry - 90, 0, 1, 0);
            glRotatef(players[k].rz, 0, 0, 1);

            glFrontFace(GL_CCW);
            glColor3f(players[k].r2, players[k].g2, players[k].b2);

            // glCallList(App::GetSingleton().graphicsTask->bodylist);
            App::GetSingleton().graphicsTask->bodylist.Call(0);

            glTranslatef(0, .10, 0);

            glRotatef(players[k].rtx, 1, 0, 0);
            glRotatef(-players[k].rty + 180, 0, 1, 0);
            glRotatef(players[k].rtz, 0, 0, 1);

            glColor3f(players[k].r, players[k].g, players[k].b);

            // glCallList(App::GetSingleton().graphicsTask->turretlist);
            App::GetSingleton().graphicsTask->turretlist.Call(0);

            glFrontFace(GL_CW);

            glPopMatrix();
            //-----

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);

            glDepthMask(GL_FALSE);  // Allow depth testing to be performed
            glDepthFunc(GL_LEQUAL); //

            glEnable(GL_BLEND); // Blend em'
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

            drift += GlobalTimer::dT;

            if (drift > 1)
                drift = 0;

            // Awesome effects are performed using the texture matrix stack. (depth = 2)
            glMatrixMode(GL_TEXTURE);

            glPushMatrix();
            // glScalef(5,5,5);
            glTranslatef(100 * drift, 0, 0);
            // glRotatef(players[k].rty+players[k].ry,0,0,1);
            glMatrixMode(GL_MODELVIEW);

            glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);

            // players[k].r=players[k].g=players[k].b=players[k].r=players[k].g2=players[k].b2=1;
            glColor3f(players[k].r, players[k].g, players[k].b);
            // players[k].Draw2();
            // glCallList(App::GetSingleton().graphicsTask->bodylist);

            //-------------
            glPushMatrix();

            // glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);

            glTranslatef(players[k].x, players[k].y + .05, players[k].z);
            glRotatef(players[k].rx, 1, 0, 0);
            glRotatef(-players[k].ry - 90, 0, 1, 0);
            glRotatef(players[k].rz, 0, 0, 1);

            glFrontFace(GL_CCW);

            // glCallList(App::GetSingleton().graphicsTask->bodylist);
            App::GetSingleton().graphicsTask->bodylist.Call(0);

            glTranslatef(0, .10, 0);

            glRotatef(players[k].rtx, 1, 0, 0);
            glRotatef(-players[k].rty + 180, 0, 1, 0);
            glRotatef(players[k].rtz, 0, 0, 1);

            glColor3f(players[k].r2, players[k].g2, players[k].b2);

            if (special[k] >= players[k].fireCost / 5)
            {
                // glCallList(App::GetSingleton().graphicsTask->turretlist);
                App::GetSingleton().graphicsTask->turretlist.Call(0);
            }

            glFrontFace(GL_CW);

            glPopMatrix();

            glMatrixMode(GL_TEXTURE); // Set matrix mode to projection

            glPopMatrix();
            glMatrixMode(GL_MODELVIEW); // Select modelview matrix

            glDepthMask(GL_TRUE);
        }

        if (tanks.size() > 0)
        {

            glDisable(GL_LIGHTING);

            glTranslatef(players[k].x, players[k].y + .25, players[k].z);
            glRotatef(-players[k].rrl, 0, 1, 0);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[20]);
            glEnable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);

            glColor4f(1.0f, players[k].dist / 50.0f, 0.1f, 1.0);

            App::GetSingleton().graphicsTask->squarelist.Call(0);

            if (players[k].fireTimer > players[k].fireRate && players[k].fireCost < players[k].charge)
            {
                glTranslatef(0, +0.018f, 0);
                glRotatef(100 * drift, 0, 1, 0);

                glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);

                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

                glScalef(0.2, 0.2, 0.2);

                App::GetSingleton().graphicsTask->squarelist.Call(0);

                glScalef(5.0, 5.0, 5.0);
            }

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glDepthMask(GL_TRUE);
            glEnable(GL_CULL_FACE);

            glEnable(GL_LIGHTING);

            glRotatef(players[k].rrl, 0, 1, 0);

            glTranslatef(-players[k].x, -players[k].y + .25, -players[k].z);
        }

        glPopMatrix();
    }

    // Draw AI tanks
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    for (const Tank &tank : tanks)
    {
        tank.Draw();
    }
}

void TankHandler::DrawBullets()
{
    glDisable(GL_TEXTURE_2D);

    for (Tank &tank : tanks)
    {
        for (Bullet &bullet : tank.bullets)
        {
            if (bullet.alive)
            {
                bullet.Draw();
            }
        }
    }

    for (int k = 0; k < numPlayers; k++)
    {
        for (Bullet &bullet : players[k].bullets)
        {
            if (bullet.alive)
            {
                bullet.Draw();
            }
        }
    }
}
