#include "TankRenderer.h"
#include "Tank.h"

void TankRenderer::Draw(const Tank& tank) {
    if (!tank.alive) {
        return;
    }
    
    glPushMatrix();
    SetupBodyTransform(tank);
    DrawBody(tank.r2, tank.g2, tank.b2, tank.energy, tank.maxEnergy);
    
    SetupBarrelTransform(tank);
    DrawBarrel(tank.r, tank.g, tank.b, tank.energy, tank.maxEnergy);
    
    SetupTurretTransform(tank);
    DrawTurret();
    
    glPopMatrix();
}

void TankRenderer::Draw2(const Tank& tank) {
    glPushMatrix();
    
    glTranslatef(tank.x, tank.y, tank.z);
    glRotatef(tank.rx, 1, 0, 0);
    glRotatef(-tank.ry, 0, 1, 0);
    glRotatef(tank.rz, 0, 0, 1);

    DrawBody(tank.r2, tank.g2, tank.b2, tank.energy, tank.maxEnergy);
    
    glRotatef(tank.rtx, 1, 0, 0);
    glRotatef(-tank.rty, 0, 1, 0);
    glRotatef(tank.rtz, 0, 0, 1);
    
    DrawBarrel(tank.r, tank.g, tank.b, tank.energy, tank.maxEnergy);
    
    glTranslatef(.1f, 0, 0);
    DrawTurret();

    glPopMatrix();
}

void TankRenderer::SetupBodyTransform(const Tank& tank) {
    glTranslatef(tank.x, tank.y, tank.z);
    glRotatef(tank.rx, 1, 0, 0);
    glRotatef(-tank.ry, 0, 1, 0);
    glRotatef(tank.rz, 0, 0, 1);
}

void TankRenderer::SetupBarrelTransform(const Tank& tank) {
    glRotatef(tank.rtx, 1, 0, 0);
    glRotatef(-tank.rty, 0, 1, 0);
    glRotatef(tank.rtz, 0, 0, 1);
}

void TankRenderer::SetupTurretTransform(const Tank& tank) {
    glTranslatef(.1f, 0, 0);
}

void TankRenderer::DrawBody(float r, float g, float b, float energy, float maxEnergy) {
    glColor3f((4 * r + (maxEnergy / energy)) / 2, (4 * g + (maxEnergy / energy)) / 2, (4 * b + (maxEnergy / energy)) / 2);
    
    glScalef(.06f, .06f, .06f);
    glBegin(GL_TRIANGLES);
    
    // Tank body geometry
    glNormal3f(0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    
    glNormal3f(0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    
    glNormal3f(-0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(-0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    
    glNormal3f(0.182574f, 0.365148f, -0.912871f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    
    glNormal3f(0.000000f, -0.447214f, -0.894427f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    
    glNormal3f(0.000000f, -0.447214f, 0.894427f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(0.182574f, 0.365148f, 0.912871f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    
    glEnd();
    glScalef(10, 10, 10);
}

void TankRenderer::DrawBarrel(float r, float g, float b, float energy, float maxEnergy) {
    glColor3f((4 * r + (maxEnergy / energy)) / 2, (4 * g + (maxEnergy / energy)) / 2, (4 * b + (maxEnergy / energy)) / 2);
    
    glScalef(.1f, .1f, .1f);
    glBegin(GL_TRIANGLES);
    
    // Tank barrel geometry
    glNormal3f(0.163846f, -0.081923f, 0.983078f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    
    glNormal3f(0.180156f, -0.041001f, 0.982783f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    
    glNormal3f(0.196116f, 0.000000f, -0.980581f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(0.180156f, -0.041001f, -0.982783f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    
    glNormal3f(-0.894427f, 0.447214f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(-0.894427f, 0.447214f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    
    glEnd();
    glScalef(10, 10, 10);
}

void TankRenderer::DrawTurret() {
    glScalef(.1f, .1f, .1f);
    glBegin(GL_TRIANGLES);
    
    // Tank turret geometry
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, -0.707107f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(5, 4, -0.25f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    
    glNormal3f(0.000000f, -0.707107f, -0.707107f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-3, 3, 0);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    
    glNormal3f(0.000000f, -0.707107f, -0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(5, 4, -0.25f);
    
    glNormal3f(-1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-3, 3, 0);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(5, 4, -0.25f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    
    glNormal3f(0.000000f, -0.707107f, 0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(5, 4, 0.25f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    
    glNormal3f(0.000000f, -0.707107f, 0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-3, 3, 0);
    
    glEnd();
    glScalef(10, 10, 10);
}
