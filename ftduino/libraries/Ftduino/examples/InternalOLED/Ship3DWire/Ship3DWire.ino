/* Ship3DWire.ino */

#include <Adafruit_GFX.h>
#include "FtduinoDisplay.h"

#include "sin.h"
#include "shipdata.h"

FtduinoDisplay display;

#define VIEW_DIST 8
#define SCALE 128

#define X 0
#define Y 1
#define Z 2

#define F2B(v)  (int8_t)(128*(v)/M_PI)

// cube rotates around all three axes
int8_t rot[3] = { 0,0,0 };
int8_t depth = 0;

void setup() { }

#define SIN(x) ((int8_t)pgm_read_byte(sintab + (uint8_t)(x)))
#define COS(x) ((int8_t)pgm_read_byte(sintab + (uint8_t)(64+(x))))

#define F16(p,e)  ((int16_t)pgm_read_word(&p->e)) 
#define F8(p,e)   ((int8_t)pgm_read_byte(&p->e)) 
#define F16U(p,e) ((uint16_t)pgm_read_word(&p->e)) 
#define F8U(p,e)  ((uint8_t)pgm_read_byte(&p->e)) 

void loop() {
  // the trusty old cobra mk3
  const ship_data *ship = &cobra3a_data;

  // space to store projected corners
  int8_t corners2d[F8U(ship,num_points)][2];

  // rotate all corners around all three axes and project them
  int16_t dist = 128*VIEW_DIST - 4*SIN(depth);

  int16_t rsin[3], rcos[3];
  for(char j=X;j<=Z;j++)
      rsin[j] = SIN(rot[j]), rcos[j] = COS(rot[j]);

  // these need to be dynamic or at least the max ever required
  // and they should be bit fields
  bool face_is_visible[F8U(ship,num_faces)];
    
  // first check which faces are visible (user faces "top")
  for(char i=0;i<F8U(ship,num_faces);i++) {
    const struct ship_face_normal *n = &((struct ship_face_normal*)pgm_read_ptr(&ship->normals))[i];
    
    // rotate normal around Z axis
    int16_t x1 =    (F8(n,x) * rcos[Z] - F8(n,y) * rsin[Z]) / 128;
    int16_t y1 =    (F8(n,x) * rsin[Z] + F8(n,y) * rcos[Z]) / 128;      
    
    // around Y axis
    int16_t z1 =    (  x1 * rsin[Y] + F8(n,z) * rcos[Y]) / 128;   
    
    // around X axis and check if z is negative
    face_is_visible[i] = (y1 * rsin[X] + z1 * rcos[X]) < 0;
  }

  // process all corners of the ship
  for(char i=0;i<F8U(ship,num_points);i++) {
    const struct ship_point *p = &((struct ship_point*)pgm_read_ptr(&ship->points))[i];

    // check if this point is part of any visible face
    if(((F8U(p,face1) >= 13) || face_is_visible[F8U(p,face1)]) ||
       ((F8U(p,face2) >= 13) || face_is_visible[F8U(p,face2)]) ||
       ((F8U(p,face3) >= 13) || face_is_visible[F8U(p,face3)]) ||
       ((F8U(p,face4) >= 13) || face_is_visible[F8U(p,face4)])) {         
      int16_t r[3];
        
      // around Z axis
      int16_t x1 = (F16(p,x) * rcos[Z] - F16(p,y) * rsin[Z]) / 128;
      int16_t y1 = (F16(p,x) * rsin[Z] + F16(p,y) * rcos[Z]) / 128;      
    
      // around Y axis
      int16_t z1 =   (x1 * rsin[Y] + F16(p,z) * rcos[Y]) / 128;   
      r[X]       =   (x1 * rcos[Y] - F16(p,z) * rsin[Y]) / 128;   
    
      // around X axis
      r[Y]       =   (y1 * rcos[X] -   z1 * rsin[X]) / 128;
      r[Z]       =   (y1 * rsin[X] +   z1 * rcos[X]) / 128;

      // project
      corners2d[i][X] = FTDDSP_WIDTH/2  + (SCALE * r[X]) / ((dist + r[Z])/2);
      corners2d[i][Y] = FTDDSP_HEIGHT/2 + (SCALE * r[Y]) / ((dist + r[Z])/2);
    }
  }
  
  // draw all lines
  display.fillScreen(BLACK);
  for(char i=0;i<F8U(ship,num_lines);i++) {
    const struct ship_line *l = &((struct ship_line*)pgm_read_ptr(&ship->lines))[i];
    if(face_is_visible[F8U(l,face1)] || face_is_visible[F8U(l,face2)]) 
      display.drawLine(corners2d[F8U(l,start_point)][X], corners2d[F8U(l,start_point)][Y],
                       corners2d[F8U(l,end_point)][X],   corners2d[F8U(l,end_point)][Y], WHITE);
  }
  display.display();

  rot[X] += F2B(0.1);
  rot[Y] += F2B(0.05);
  rot[Z] += F2B(0.03);
  depth  += F2B(0.05);
}


