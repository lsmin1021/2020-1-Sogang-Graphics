#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>
#include "Shaders/LoadShaders.h"
#include "my_geom_objects.h"


GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_GS; // handles to shader programs

// for simple shaders
GLint loc_ModelViewProjectionMatrix_simple;

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4 
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_texture, loc_flag_texture_mapping, loc_flag_fog, loc_flag_godzilla, loc_flag_wolf,loc_flag_shadow;
GLint loc_color_timer;
GLint loc_screen_flag;
GLfloat loc_screen_f, loc_screen_r;


// for Gouraud Shading (Textured) shaders
GLint loc_global_ambient_color_G;
loc_light_Parameters loc_light_G[NUMBER_OF_LIGHT_SUPPORTED];
GLint loc_ModelViewProjectionMatrix_GS, loc_ModelViewMatrix_GS, loc_ModelViewMatrixInvTrans_GS;
GLint loc_texture_G, loc_flag_texture_mapping_G, loc_flag_fog_G;



int change_shader = 1; //Phong인지 Gouraud인지 판단하는 flag. 1이면 phong
int flag_texture_mapping, flag_godzilla, flag_wolf, flag_shadow;
int color_timer;


//screen 효과를 위한 변수들
int flag_draw_screen;

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.
glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ViewMatrix, ProjectionMatrix,ViewProjectionMatrix;


// lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];


/*카메라 코드 시작*/


void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(camera.uaxis[0], camera.vaxis[0], camera.naxis[0], 0.0f,
		camera.uaxis[1], camera.vaxis[1], camera.naxis[1], 0.0f,
		camera.uaxis[2], camera.vaxis[2], camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-camera.pos[0], -camera.pos[1], -camera.pos[2]));

}

#define CAM_RSPEED 0.1f
void renew_cam_orientation_rotation_around_v_axis(int angle) {
	// let's get a help from glm
	glm::mat3 RotationMatrix;
	glm::vec3 direction;

	RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
		glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2])));

	direction = RotationMatrix * glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2]);
	camera.uaxis[0] = direction.x; camera.uaxis[1] = direction.y; camera.uaxis[2] = direction.z;
	direction = RotationMatrix * glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2]);
	camera.naxis[0] = direction.x; camera.naxis[1] = direction.y; camera.naxis[2] = direction.z;
}
void renew_cam_orientation_rotation_around_u_axis(int angle) {
	// let's get a help from glm
	glm::mat3 RotationMatrix;
	glm::vec3 direction;
	RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
		glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2])));

	direction = RotationMatrix * glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2]);
	camera.vaxis[0] = direction.x; camera.vaxis[1] = direction.y; camera.vaxis[2] = direction.z;
	direction = RotationMatrix * glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2]);
	camera.naxis[0] = direction.x; camera.naxis[1] = direction.y; camera.naxis[2] = direction.z;
}
void renew_cam_orientation_rotation_around_n_axis(int angle) {
	// let's get a help from glm
	glm::mat3 RotationMatrix;
	glm::vec3 direction;

	RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
		glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2])));

	direction = RotationMatrix * glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2]);
	camera.vaxis[0] = direction.x; camera.vaxis[1] = direction.y; camera.vaxis[2] = direction.z;
	direction = RotationMatrix * glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2]);
	camera.uaxis[0] = direction.x; camera.uaxis[1] = direction.y; camera.uaxis[2] = direction.z;
}
void initialize_camera(void) {
	camera.pos[0] = 600.0f; camera.pos[1] = 300.0f;  camera.pos[2] = 500.0f;
	camera.uaxis[0] = 0.8f; camera.uaxis[1] = 0.0f; camera.uaxis[2] = -1.0f;
	camera.vaxis[0] = -0.5f; camera.vaxis[2] = 0.0f; camera.vaxis[1] = 1.0f;
	camera.naxis[1] = 0.0f;  camera.naxis[2] = 0.0f; camera.naxis[0] = 1.0f;
	camera.move = 0;
	camera.fovy = 40.0f, camera.aspect_ratio = 1.0f; camera.near_c = 0.1f; camera.far_c = 2000.0f;
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

enum axes { X_AXIS, Y_AXIS, Z_AXIS };
int flag_translation_axis;
#define CAM_TSPEED 0.05f
void renew_cam_position(int del) {
	switch (flag_translation_axis) {
	case X_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (camera.uaxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (camera.uaxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (camera.uaxis[2]);
		break;
	case Y_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (camera.vaxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (camera.vaxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (camera.vaxis[2]);
		break;
	case Z_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (-camera.naxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (-camera.naxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (-camera.naxis[2]);
		break;
	}
}




/*카메라 끝*/





// callbacks
float PRP_distance_scale[6] = { 0.5f, 1.0f, 2.5f, 5.0f, 10.0f, 20.0f };


unsigned int timestamp_bike = 0, timestamp_ben = 0;

int dragon_tex_flag = 1;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 ModelMatrix_Dragon;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);

	glUseProgram(h_ShaderProgram_TXPS);
  	set_material_floor();
	glUniform1i(loc_texture, TEXTURE_ID_FLOOR);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500.0f, 0.0f, 500.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_floor();
	
	//정적인 물체 위치 시키기

	//용 시작
	set_material_tiger();
	if(dragon_tex_flag)
		glUniform1i(loc_texture, TEXTURE_ID_TIGER);
	else
		glUniform1i(loc_texture, TEXTURE_ID_LUNA);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-150.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(4.0f, 4.0f, 5.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_dragon();
	//용 끝

	//오토바이 시작
	float bike_x, bike_z, bike_clock;
	bike_clock = (timestamp_bike % 1442);
	bike_clock = bike_clock / 2;
	bike_clock -= 360;
	float bike_scale = 2 / (3 - cosf(2 * bike_clock*TO_RADIAN));
	bike_x = bike_scale*cosf(bike_clock*TO_RADIAN) * 350.0f;
	bike_z = bike_scale*sinf(bike_clock * TO_RADIAN * 2) / 2 * 350.0f;
	float bike_angle= 90.0f * TO_RADIAN - cosf(bike_clock * TO_RADIAN);
	set_material_tiger();
	glUniform1i(loc_texture, TEXTURE_ID_BIKE);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(bike_x, 0.0f, bike_z));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, bike_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_bike();
	//오토바이 끝


	//고질라 시작
	if (change_shader == 1) { //phong
		glUseProgram(h_ShaderProgram_TXPS);
		flag_godzilla = 1;
		glUniform1i(loc_flag_godzilla, flag_godzilla);
		set_material_godzilla_P();
		glUniform1i(loc_texture, 0);
	}
	else { //gouraud
		glUseProgram(h_ShaderProgram_GS);
		set_material_godzilla_G();
	}
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(150.0f, 0.0f, 00.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	if (change_shader == 1) { //phong
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		draw_godzilla();
		flag_godzilla = 0;
		flag_wolf = 0;
		glUniform1i(loc_flag_godzilla, flag_godzilla);
		glUniform1i(loc_flag_wolf, flag_wolf);
	}
	else { //gouraud
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		draw_godzilla();
	}
	glUseProgram(0);
	//고질라 끝

	
	//동적인 물체 위치	
	glUseProgram(h_ShaderProgram_TXPS);
	//벤 시작
	float ben_y, ben_z, ben_clock;
	ben_clock = (timestamp_ben % 1442);
	ben_clock = ben_clock/ 2 - 360;
	ben_z = sinf(ben_clock * TO_RADIAN)*(100.0f);
	ben_y = cosf(ben_clock * TO_RADIAN)* (100.0f);
	float ben_height;
	ben_height = ((int)ben_clock) % 100;
	if(ben_height < 0)
		ben_height *= -1;
	
	if (ben_height > 50)
		ben_height = 50 - ((int) ben_height)%50;
	set_material_ben();
	glUniform1i(loc_texture, TEXTURE_ID_BEN);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, ben_y, ben_z));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, ben_height - 100.0f, -100.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, ben_clock * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		
	glUniform1i(loc_screen_flag, flag_draw_screen);
	draw_ben();
	glUniform1i(loc_screen_flag, 0);
	//벤 끝

	//거미 시작
	float spider_clock, spider_x, spider_y, spider_angle;
	spider_clock = ben_clock;
	spider_x = sinf(spider_clock * TO_RADIAN) * 100.0f;
	spider_y = cosf(spider_clock * TO_RADIAN) * 100.0f;
	spider_angle = (((int)spider_clock) % 360) * TO_RADIAN;
	float minus_flag = 1;
	if (spider_y < 0) {
		spider_y = spider_y * -1 + 90.0f;
		minus_flag = -1;
	}
	else
		spider_y += 90.0f;
	light[3].position[0] = spider_x;
	light[3].position[2] = spider_x;
	light[3].position[2] = 0;
	light[3].position[3] = 1.0f;

	light[3].spot_direction[0] = 0.0f;
	light[3].spot_direction[1] = -1.0f;
	light[3].spot_direction[2] = 0.0f;
	set_material_spider();
	glUniform1i(loc_texture, TEXTURE_ID_SPIDER);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(spider_x, spider_y, 0.0f));
	//거미에 고정된 광원
	glm::vec4 position_EC_3 = ModelViewMatrix * glm::vec4(light[3].position[0], light[3].position[1],
		light[3].position[2], light[3].position[3]);
	position_EC_3[1] -= 30;
	glUniform4fv(loc_light[3].position, 1, &position_EC_3[0]);
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f*TO_RADIAN * minus_flag, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, spider_angle, glm::vec3(1.0f, 0.0f,  0.0f));	
	if(minus_flag== -1)
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 direction_EC_3 = glm::mat3(ModelViewMatrix) * glm::vec3(light[3].spot_direction[0], light[3].spot_direction[1],
		light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC_3[0]);

	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(30.0f, -30.0f, 30.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_spider();
	//거미 끝

	//늑대 시작
	float wolf_clock;
	wolf_clock = timestamp_ben%720;
	float wolf_z, wolf_scale;
	wolf_z = wolf_clock / 2 - 180;
	wolf_scale = wolf_clock /72;
	if (wolf_scale > 5)
		wolf_scale = 10 - wolf_scale;
	
	if (flag_shadow) {
		if (wolf_clock < 45)
			color_timer = 0;
		else if (wolf_clock < 90)
			color_timer = 1;
		else if (wolf_clock < 135)
			color_timer = 2;
		else if(wolf_clock < 180)
			color_timer = 3;
		else if (wolf_clock < 225)
			color_timer = 4;
		else if (wolf_clock < 270) 
			color_timer = 5;
		else if (wolf_clock < 315)
			color_timer = 6;
		else if (wolf_clock < 360)
			color_timer = 7;
		else if (wolf_clock < 405)
			color_timer = 0;
		else if (wolf_clock < 450)
			color_timer = 1;
		else if (wolf_clock < 495)
			color_timer = 2;
		else if (wolf_clock < 540)
			color_timer = 3;
		else if (wolf_clock < 585)
			color_timer = 4;
		else if (wolf_clock < 630)
			color_timer = 5;
		else if (wolf_clock < 675)
			color_timer = 6;
		else 
			color_timer = 7;
		glUniform1i(loc_color_timer, color_timer);
	}
	set_material_wolf();
	flag_wolf = 1;
	glUniform1i(loc_flag_wolf, flag_wolf);
	glUniform1i(loc_texture, TEXTURE_ID_WOLF);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, wolf_z));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(30.0f*wolf_scale, 30.0f * wolf_scale, 30.0f * wolf_scale));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wolf();	
	flag_wolf = 0;
	glUniform1i(loc_flag_wolf, 0);
	//늑대 끝

	glUseProgram(0);

	glutSwapBuffers();
}

void timer_scene(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	timestamp_bike = timestamp_scene;
	timestamp_ben = timestamp_scene;
	cur_frame_ben = timestamp_scene % N_BEN_FRAMES;
	cur_frame_wolf= timestamp_scene % N_WOLF_FRAMES;
	cur_frame_spider = timestamp_scene % N_SPIDER_FRAMES;
	rotation_angle_tiger = (timestamp_scene % 360)*TO_RADIAN;
	glutPostRedisplay();
	if (flag_tiger_animation)
		glutTimerFunc(10, timer_scene, 0);
}

float screen_f = 1.0f, screen_r = 0.1f;
#define SCREEN_MAX_FL 50.0f

void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 0;
	static int PRP_distance_level = 4;

	glm::vec4 position_EC;
	glm::vec3 direction_EC;

	if ((key >= '0') && (key <= '0' + NUMBER_OF_LIGHT_SUPPORTED - 1)) { //0,1,2,3 광원 on/off
		int light_ID = (int)(key - '0');

		glUseProgram(h_ShaderProgram_TXPS);
		light[light_ID].light_on = 1 - light[light_ID].light_on;
		glUniform1i(loc_light[light_ID].light_on, light[light_ID].light_on);
		glUseProgram(0);

		glUseProgram(h_ShaderProgram_GS);
		glUniform1i(loc_light_G[light_ID].light_on, light[light_ID].light_on);
		glUseProgram(0);


		glutPostRedisplay();
		return;
	}

	switch (key) {
	case 'a': // toggle the animation effect.
		flag_tiger_animation = 1 - flag_tiger_animation;
		if (flag_tiger_animation) {
			glutTimerFunc(100, timer_scene, 0);
			fprintf(stdout, "^^^ Animation mode ON.\n");
		}
		else
			fprintf(stdout, "^^^ Animation mode OFF.\n");
		break;
	case 'e': //사람에 구멍 뚫기
		flag_draw_screen = 1;
		if (flag_draw_screen) {
			screen_f += 1.0f;
			if (screen_f > SCREEN_MAX_FL) {
				screen_f = SCREEN_MAX_FL;
			}
			glUseProgram(h_ShaderProgram_TXPS);
			glUniform1f(loc_screen_f, screen_f);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	case 't': //용 텍스쳐 변경
		dragon_tex_flag = 1 - dragon_tex_flag;
		glutPostRedisplay();
		break;
	case 'p':
		flag_polygon_fill = 1 - flag_polygon_fill;
		if (flag_polygon_fill)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glutPostRedisplay();
		break;
	case '+': //zoom-in
		if (camera.fovy > 20) {
			camera.fovy -= 1;
			ProjectionMatrix = glm::perspective(camera.fovy * TO_RADIAN, camera.aspect_ratio, camera.near_c, camera.far_c);
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case '-': //zoom-out
		if (camera.fovy < 50) {
			camera.fovy += 1;
			ProjectionMatrix = glm::perspective(camera.fovy * TO_RADIAN, camera.aspect_ratio, camera.near_c, camera.far_c);
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'q': //phone과 gouraud 번갈아 적용
		change_shader = 1 - change_shader;
		glutPostRedisplay();
		break;
	case 'w': //늑대 쉐이더 변경
		flag_shadow = 1 - flag_shadow;

		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1f(loc_flag_shadow, flag_shadow);
		glUniform1d(loc_color_timer, color_timer);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups
		break;
	}
}

int prevx, prevy;

int mouse_press = 0, left_press = 0, right_press = 0;
void motion(int x, int y) {
	if (!camera.move) return;
	if (!mouse_press) return;
	if (left_press == 1) {
		renew_cam_orientation_rotation_around_v_axis(prevx - x);
		renew_cam_orientation_rotation_around_u_axis(prevy - y);
	}
	else if (right_press == 1) {
		renew_cam_position(prevy - y);
		renew_cam_orientation_rotation_around_n_axis(prevx - x);
	}

	prevx = x; prevy = y;

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}
void mouse(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON)) {
		if (state == GLUT_DOWN) {
			left_press = 1;
			mouse_press = 1;
			camera.move = 1;
			prevx = x; prevy = y;
		}
		else if (state == GLUT_UP) {
			left_press = 0;
			mouse_press = 0;
			camera.move = 0;
		}
	}
	if ((button == GLUT_RIGHT_BUTTON)) {
		if (state == GLUT_DOWN) {
			right_press = 1;
			mouse_press = 1;
			camera.move = 1;
			flag_translation_axis = Z_AXIS;
			prevx = x; prevy = y;
		}
		else if (state == GLUT_UP) {
			right_press = 0;
			mouse_press = 0;
			camera.move = 0;
		}
	}
}
void special(int key, int x, int y) {
#define SENSITIVITY 5.0
	switch (key) {
	case GLUT_KEY_LEFT:
		camera.move = 1;
		prevx -= SENSITIVITY;
		flag_translation_axis = X_AXIS;
		renew_cam_position(prevx - x);
		prevx = x; prevy = y;
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		camera.move = 1;
		prevx += SENSITIVITY;
		flag_translation_axis = X_AXIS;
		renew_cam_position(prevx - x);
		prevx = x; prevy = y;
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		camera.move = 1;
		prevy -= SENSITIVITY;
		flag_translation_axis = Y_AXIS;
		renew_cam_position(prevy - y);
		prevx = x; prevy = y;
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		camera.move = 1;
		prevy += SENSITIVITY;
		flag_translation_axis = Y_AXIS;
		renew_cam_position(prevy - y);
		prevx = x; prevy = y;
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	default:
		camera.move = 0;
		break;
	}
}
void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	camera.aspect_ratio = (float) width / height;
	ProjectionMatrix = glm::perspective(camera.fovy*TO_RADIAN, camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO); 
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &rectangle_VAO);
	glDeleteBuffers(1, &rectangle_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);

	glDeleteTextures(N_TEXTURES_USED, texture_names);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}


void prepare_shader_program(void) {
	int i;
	char string[256];
	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_TXPS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
		{ GL_NONE, NULL }
	};


	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");

	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");

	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");
	loc_flag_godzilla = glGetUniformLocation(h_ShaderProgram_TXPS, "u_is_godzilla");
	loc_flag_wolf = glGetUniformLocation(h_ShaderProgram_TXPS, "u_is_wolf");
	loc_flag_shadow = glGetUniformLocation(h_ShaderProgram_TXPS, "u_shadow_effect");
	loc_color_timer = glGetUniformLocation(h_ShaderProgram_TXPS, "u_color_timer");

	loc_screen_flag = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_flag");
	loc_screen_f = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_f");
	loc_screen_r = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_r");

	ShaderInfo shader_info_GS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
		{ GL_NONE, NULL }
	};
	h_ShaderProgram_GS = LoadShaders(shader_info_GS);
	loc_ModelViewProjectionMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color_G = glGetUniformLocation(h_ShaderProgram_GS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light_G[i].light_on = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light_G[i].position = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light_G[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light_G[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light_G[i].specular_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light_G[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light_G[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light_G[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light_G[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GS, string);
	}

	loc_material_G.ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.ambient_color");
	loc_material_G.diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.diffuse_color");
	loc_material_G.specular_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_color");
	loc_material_G.emissive_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.emissive_color");
	loc_material_G.specular_exponent = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_exponent");
	loc_flag_texture_mapping_G = glGetUniformLocation(h_ShaderProgram_GS, "u_flag_texture_mapping");
	
	loc_texture_G = glGetUniformLocation(h_ShaderProgram_GS, "u_base_texture");
}

void initialize_lights_and_material(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 0.115f, 0.115f, 0.115f, 1.0f);
	
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		if(i == 0 || i == 1)
			glUniform1i(loc_light[i].light_on, 1); //0, 1 번 광원만 키기
		else
			glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 300.0f, -1.0f, 0.0f);
		if (i == 3) {
				glUniform4f(loc_light[i].position, 0.0f, 300.0f, 0.0f, 1.0f);
		}
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 0.4f, 0.5f, 0.3f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.4f, 0.5f, 0.3f, 1.0f);
		}
		else if (i == 1) {
			glUniform4f(loc_light[i].diffuse_color, 0.3f, 0.2f, 0.4f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.3f, 0.2f, 0.4f, 1.0f);
		}
		else if (i == 3) {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 1.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		if (i == 3) {
			glUniform3f(loc_light[i].spot_direction, 0.0f, -1.0f, 0.0f);
			glUniform1f(loc_light[i].spot_exponent, 8.0f); // [0.0, 128.0]
			glUniform1f(loc_light[i].spot_cutoff_angle, 20.0f);// [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		}
		else {
			glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, 1.0f);
			glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
			glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f);// [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		}
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}
	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]

	glUseProgram(0);

	glUseProgram(h_ShaderProgram_GS);
	glUniform4f(loc_global_ambient_color_G, 0.115f, 0.115f, 0.115f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		if (i == 0 || i == 1)
			glUniform1i(loc_light_G[i].light_on, 1); //0, 1 번 광원만 키기
		else
			glUniform1i(loc_light_G[i].light_on, 0); // turn off all lights initially

		glUniform4f(loc_light_G[i].position, 0.0f, 300.0f, -1.0f, 0.0f);
		if (i == 3) {
			glUniform4f(loc_light_G[i].position, 0.0f, 300.0f, 0.0f, 1.0f);
		}
		glUniform4f(loc_light_G[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light_G[i].diffuse_color, 0.4f, 0.5f, 0.3f, 1.0f);
			glUniform4f(loc_light_G[i].specular_color, 0.4f, 0.5f, 0.3f, 1.0f);
		}
		else if (i == 1) {
			glUniform4f(loc_light_G[i].diffuse_color, 0.3f, 0.2f, 0.4f, 1.0f);
			glUniform4f(loc_light_G[i].specular_color, 0.3f, 0.2f, 0.4f, 1.0f);
		}
		else if (i == 3) {
			glUniform4f(loc_light_G[i].diffuse_color, 0.0f, 0.0f, 1.0f, 1.0f);
			glUniform4f(loc_light_G[i].specular_color, 0.0f, 0.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light_G[i].diffuse_color, 0.0f, 1.0f, 0.0f, 1.0f);
			glUniform4f(loc_light_G[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		if (i == 3) {
			glUniform3f(loc_light_G[i].spot_direction, 0.0f, -1.0f, 0.0f);
			glUniform1f(loc_light_G[i].spot_exponent, 8.0f); // [0.0, 128.0]
			glUniform1f(loc_light_G[i].spot_cutoff_angle, 20.0f);// [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		}
		else {
			glUniform3f(loc_light_G[i].spot_direction, 0.0f, 0.0f, 1.0f);
			glUniform1f(loc_light_G[i].spot_exponent, 0.0f); // [0.0, 128.0]
			glUniform1f(loc_light_G[i].spot_cutoff_angle, 180.0f);// [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		}
		glUniform4f(loc_light_G[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}
	glUniform4f(loc_material_G.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material_G.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material_G.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material_G.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material_G.specular_exponent, 0.0f); // [0.0, 128.0]

	glUseProgram(0);
}

void initialize_flags(void) {
	flag_tiger_animation = 1;
	flag_polygon_fill = 1;
	flag_texture_mapping = 1;
	change_shader = 1;
	flag_godzilla = 0;
	flag_wolf = 0;
	flag_shadow = 0;
	flag_draw_screen = 0;
	color_timer = 1;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_screen_flag, flag_draw_screen);
	glUniform1i(loc_flag_texture_mapping, flag_texture_mapping);
	glUniform1i(loc_flag_godzilla, flag_godzilla);
	glUniform1i(loc_flag_wolf, flag_wolf);
	glUniform1i(loc_flag_shadow, flag_shadow);
	glUniform1i(loc_color_timer, color_timer);
	glUseProgram(0);

	glUseProgram(h_ShaderProgram_GS);
	glUniform1i(loc_flag_texture_mapping_G, flag_texture_mapping);
	glUseProgram(0);

}

void initialize_OpenGL(void) {

	glEnable(GL_MULTISAMPLE);


  	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	initialize_lights_and_material();
	initialize_flags();

	flag_translation_axis = Z_AXIS; // 카메라 움직이는 축 z축으로 초기화

	glGenTextures(N_TEXTURES_USED, texture_names);
}

void set_up_scene_lights(void) {
	// point_light_EC: use light 0
	
	light[0].light_on = 1; light[1].light_on = 1;

	light[2].position[0] = 0.0f; light[2].position[1] = 500.0f; 	// point light position in EC
	light[2].position[2] = 0.0f; light[2].position[3] = 1.0f;

	light[2].ambient_color[0] = 1.0; light[2].ambient_color[1] = 1.0f;
	light[2].ambient_color[2] = 0.13f; light[2].ambient_color[3] = 1.0f;

	light[2].diffuse_color[0] = 1.0f; light[2].diffuse_color[1] = 0.0f;
	light[2].diffuse_color[2] = 0.0f; light[2].diffuse_color[3] = 1.0f;

	light[2].specular_color[0] = 1.0f; light[2].specular_color[1] = 0.0f;
	light[2].specular_color[2] = 0.0f; light[2].specular_color[3] = 1.0f;

	light[2].spot_direction[0] = camera.uaxis[0]; light[2].spot_direction[1] = camera.naxis[1]; // spot light direction in WC
	light[2].spot_direction[2] = camera.naxis[2];
	light[2].spot_cutoff_angle = 10.0f;
	light[2].spot_exponent = 8.0f;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_light[2].light_on, light[2].light_on);
	glm::vec4 position_EC = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1],
		light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light[2].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light[2].specular_color, 1, light[2].specular_color);
	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[0].spot_direction[0], light[0].spot_direction[1],
		light[0].spot_direction[2]);
	glUniform3fv(loc_light[2].spot_direction, 1, &direction_EC[2]);
	glUniform1f(loc_light[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light[2].spot_exponent, light[2].spot_exponent);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glUniform1i(loc_light[3].light_on, light[3].light_on);
	
	glUseProgram(0);
	glUseProgram(h_ShaderProgram_GS);
	glUniform1i(loc_light_G[2].light_on, light[2].light_on);
	glm::vec4 position_EC_G = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1],
		light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light_G[2].position, 1, &position_EC_G[0]);
	glUniform4fv(loc_light_G[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light_G[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light_G[2].specular_color, 1, light[2].specular_color);
	glm::vec3 direction_EC_G = glm::mat3(ViewMatrix) * glm::vec3(light[0].spot_direction[0], light[0].spot_direction[1],
		light[0].spot_direction[2]);
	glUniform3fv(loc_light_G[2].spot_direction, 1, &direction_EC_G[2]);
	glUniform1f(loc_light_G[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light_G[2].spot_exponent, light[2].spot_exponent);
	glUniform1i(loc_light_G[0].light_on, light[0].light_on);
	glUniform1i(loc_light_G[1].light_on, light[1].light_on);
	glUniform1i(loc_light_G[3].light_on, light[3].light_on);

	glUseProgram(0);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_floor();
	prepare_tiger();
	prepare_ben();
	prepare_wolf();
	prepare_luna_dragon();
	prepare_spider();
	prepare_dragon();
	prepare_bike();
	prepare_godzilla();
	set_up_scene_lights();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 3D Objects";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: '0', '1', '2', '3', 'a', 't', 'e', 'q', 'w', 'ESC'"  };

	glutInit(&argc, argv);
  	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 800);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}