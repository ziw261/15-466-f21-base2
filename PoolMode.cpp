#include "PoolMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint pool_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("pool_ver2.pnct"));
	pool_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > pool_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("pool_ver2.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = pool_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PoolMode::PoolMode() : scene(*pool_scene) {

	loadObjects();

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PoolMode::~PoolMode() {
}

void PoolMode::loadObjects() {
	for (auto& transform : scene.transforms) {
		if (transform.name == "Black_Ball")
			balls.push_back(Ball(Ball_Color::Black, false, &transform));
		else if (transform.name == "Blue_Ball")
			balls.push_back(Ball(Ball_Color::Blue, false, &transform));
		else if (transform.name == "Brown_Ball")
			balls.push_back(Ball(Ball_Color::Brown, false, &transform));
		else if (transform.name == "Blue_Flower")
			balls.push_back(Ball(Ball_Color::Blue, true, &transform));
		else if (transform.name == "Brown_Flower")
			balls.push_back(Ball(Ball_Color::Brown, true, &transform));
		else if (transform.name == "Green_Ball")
			balls.push_back(Ball(Ball_Color::Green, false, &transform));
		else if (transform.name == "Green_Flower")
			balls.push_back(Ball(Ball_Color::Green, true, &transform));
		else if (transform.name == "Orange_Ball")
			balls.push_back(Ball(Ball_Color::Orange, false, &transform));
		else if (transform.name == "Orange_Flower")
			balls.push_back(Ball(Ball_Color::Orange, true, &transform));
		else if (transform.name == "Pink_Ball")
			balls.push_back(Ball(Ball_Color::Pink, false, &transform));
		else if (transform.name == "Pink_Flower")
			balls.push_back(Ball(Ball_Color::Pink, true, &transform));
		else if (transform.name == "Red_Ball")
			balls.push_back(Ball(Ball_Color::Red, false, &transform));
		else if (transform.name == "Red_Flower")
			balls.push_back(Ball(Ball_Color::Red, true, &transform));
		else if (transform.name == "Yellow_Ball")
			balls.push_back(Ball(Ball_Color::Yellow, false, &transform));
		else if (transform.name == "Yellow_Flower")
			balls.push_back(Ball(Ball_Color::Yellow, true, &transform));
		else if (transform.name == "Player")
			player = Player(&transform);
	}

	assert(player.transform != nullptr);
	assert(balls.size() == 15);
}

bool PoolMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} if (evt.key.keysym.sym == SDLK_w) {
			cam_zoomin.downs += 1;
			cam_zoomin.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			cam_zoomout.downs += 1;
			cam_zoomout.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		}
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_w) {
			cam_zoomin.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			cam_zoomout.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		}
	}

	return false;
}

void PoolMode::update(float elapsed) {

	//move player:
	update_player_movement(elapsed);

	//move camera:
	update_camera(elapsed);
}

void PoolMode::update_player_movement(float elapsed) {
	constexpr float PlayerSpeed = 1.0f;
	glm::vec2 player_move = glm::vec2(0.0f);
	if (up.pressed) player_move.y += 1.0f;
	if (left.pressed) player_move.x += 1.0f;
	if (down.pressed) player_move.y -= 1.0f;
	if (right.pressed) player_move.x -= 1.0f;

	if (player_move != glm::vec2(0.0f)) player_move = glm::normalize(player_move) * PlayerSpeed * elapsed;

	glm::mat4x3 frame = player.transform->make_local_to_parent();
	glm::vec3 forward_dir = frame[0];
	glm::vec3 left_dir = frame[1];
	player.transform->position += player_move.x * left_dir + player_move.y * forward_dir;

	up.downs = 0;
	down.downs = 0;
	left.downs = 0;
	right.downs = 0;
}

void PoolMode::update_camera(float elapsed) {
	//combine inputs into a move:
	glm::vec2 cam_move = glm::vec2(0.0f);
	if (cam_zoomout.pressed && !cam_zoomin.pressed) cam_move.y = -1.0f;
	if (!cam_zoomout.pressed && cam_zoomin.pressed) cam_move.y = 1.0f;

	glm::mat4x3 frame = camera->transform->make_local_to_parent();
	//glm::vec3 up = frame[1];
	glm::vec3 forward = -frame[2];

	camera->transform->position += cam_move.y * forward;

	cam_zoomin.downs = 0;
	cam_zoomout.downs = 0;
}

void PoolMode::update_player_collision(glm::vec2 movement) {
	//float xmin = player.transform->position - 
}

void PoolMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
