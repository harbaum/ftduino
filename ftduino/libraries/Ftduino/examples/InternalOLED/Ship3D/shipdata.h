#ifndef SHIPDATA_H
#define SHIPDATA_H

struct ship_point
{
	int x;
	int y;
	int z;
	uint8_t dist;
	uint8_t face1;
	uint8_t face2;
	uint8_t face3;
	uint8_t face4;
};


struct ship_line
{
	uint8_t dist;
	uint8_t face1;
	uint8_t face2;
	uint8_t start_point;
	uint8_t end_point;
};


struct ship_face_normal
{
	uint8_t dist;
	int8_t x;
	int8_t y;
	int8_t z;
};

struct ship_data
{
	char name[32];
	uint8_t num_points;
	uint8_t num_lines;
	uint8_t num_faces;  
	int max_loot;
	int scoop_type;
	int size;              // double size;
	int front_laser;
	int bounty;
	int vanish_point;
	int energy;
	int velocity;
	int missiles;
	int laser_strength;
	const struct ship_point *points;
	const struct ship_line *lines;
	const struct ship_face_normal *normals;
};

extern const struct ship_data cobra3a_data;

#endif
