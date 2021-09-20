#include "PoolMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

static std::mt19937 mt(std::random_device{}());


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
	create_walls();
	pick_next_target();

	std::cout << "Wall: " <<
		wall[0] << " " <<
		wall[1] << " " <<
		wall[2] << " " <<
		wall[3] << " " <<
		std::endl;

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
		else if (transform.name == "Goal.N")
			goals.push_back(Goal(&transform));
		else if (transform.name == "Goal.NE")
			goals.push_back(Goal(&transform));
		else if (transform.name == "Goal.NW")
			goals.push_back(Goal(&transform));
		else if (transform.name == "Goal.S")
			goals.push_back(Goal(&transform));
		else if (transform.name == "Goal.SE")
			goals.push_back(Goal(&transform));
		else if (transform.name == "Goal.SW")
			goals.push_back(Goal(&transform));
	}

	assert(player.transform != nullptr);
	assert(balls.size() == 15);
	assert(goals.size() == 6);
}

void PoolMode::create_walls() {
	float xmin = FLT_MAX;
	float xmax = FLT_MIN;
	float ymin = FLT_MAX;
	float ymax = FLT_MIN;

	for (const auto& g : goals) {
		glm::vec3 pos = g.transform->position;
		xmin = xmin > pos.y ? pos.y : xmin;
		xmax = xmax < pos.y ? pos.y : xmax;
		ymin = ymin > pos.x ? pos.x : ymin;
		ymax = ymax < pos.x ? pos.x : ymax;
	}

	wall = glm::vec4(xmin, xmax, ymin, ymax);
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

	// GameState
	if (update_gamestate()) return;

	//player:
	update_player_movement(elapsed);

	//ball:
	update_ball_movement(elapsed);

	//camera:
	update_camera(elapsed);

	//collision
	reset_collision_cooldown();

	for(auto& b : balls) {
		check_collision_bb(b);
		check_collision_bp(b, player);
		check_collision_bw(b);
	}

	// Hit Goal and delete this ball
	for (size_t i=0; i<balls.size(); i++) {
		if (check_collision_bg(balls[i])) {
			if (i != next_target_idx) {
				//std::cout << "Game Over" << std::endl;
				is_game_over = true;
			}
			else {
				std::swap(balls[i], balls[balls.size()-1]);
				balls.pop_back();
				for (auto it = scene.drawables.begin(); it != scene.drawables.end(); ++it) {
					if ((*it).transform->name == next_target) {
						scene.drawables.erase(it);
					}
				}
				if (balls.size() != 0)
					pick_next_target();
				else
					is_game_over = true;
			}
		}
	}
}

bool PoolMode::update_gamestate() {
	if (is_game_over) {
		if (balls.size() != 0) {
			next_target = "You lose! Press R to restart.";
		}
		else {
			next_target = "You win! Press R to play again.";
		}
		return false;
	}
	return false;
}

void PoolMode::pick_next_target() {
	next_target_idx = mt() % (balls.size());
	next_target = balls[next_target_idx].transform->name;
}

void PoolMode::update_player_movement(float elapsed) {
	glm::vec2 player_move = glm::vec2(0.0f);
	if (up.pressed) player_move.y += 1.0f;
	if (left.pressed) player_move.x += 1.0f;
	if (down.pressed) player_move.y -= 1.0f;
	if (right.pressed) player_move.x -= 1.0f;

	if (player_move != glm::vec2(0.0f)) player_move = glm::normalize(player_move) * player_speed * elapsed;

	glm::vec3 movement = glm::vec3(player_move.y, player_move.x, 0);
	player.transform->position += movement;

	up.downs = 0;
	down.downs = 0;
	left.downs = 0;
	right.downs = 0;

	//clamp player within table:
	auto& pos = player.transform->position;
	pos.x = std::max(pos.x, wall[2] + player.size.y * 0.5f);
	pos.x = std::min(pos.x, wall[3] - player.size.y * 0.5f);
	pos.y = std::max(pos.y, wall[0] + player.size.x * 0.5f);
	pos.y = std::min(pos.y, wall[1] - player.size.x * 0.5f);

	//if (player_move != glm::vec2(0.0f)) 	
	//	std::cout << "Player Position: " <<
	//		player.transform->position.x <<
	//		" " << player.transform->position.y <<
	//		" " << player.transform->position.z <<
	//		std::endl;
}

void PoolMode::update_ball_movement(float elapsed) {
	for (auto& b : balls) {
		if (b.speed <= 0) continue;
		glm::vec2 ball_move = b.move_dir * b.speed * elapsed;
		glm::vec3 old_pos = b.transform->position;
		glm::vec3 movement = glm::vec3(ball_move.y, ball_move.x, 0);
		b.transform->position += movement;
		b.speed = b.speed ? b.speed - SPEED_DECAY * elapsed : 0;
		b.move_dir = b.speed ? b.move_dir : glm::vec2(0.0f);


		float distance = glm::length(movement);
		glm::vec3 real_dir = glm::normalize(old_pos - b.transform->position);
		glm::vec3 rotate_axis = glm::normalize(glm::cross(real_dir, glm::vec3(0,0,1)));
		float angle = (distance / (b.size.x * 0.5f));
		b.transform->rotation =
			glm::normalize(b.transform->rotation * glm::angleAxis(angle, rotate_axis));
	}
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

void PoolMode::reset_collision_cooldown() {
	for (size_t i = 0; i < balls.size(); i++) {
		for (size_t j = 0; j < balls.size(); j++) {
			if (i == j) continue;
			float distance = glm::distance(balls[i].transform->position, balls[j].transform->position);
			if (std::abs(distance) > balls[i].size.x + COLLISION_OFFSET) {
				balls[i].is_in_collision = false;
				balls[j].is_in_collision = false;
			}
			else if (std::abs(distance) <= balls[i].size.x - COLLISION_OFFSET){
				if (balls[i].speed <= 0 && balls[j].speed <= 0) {
					balls[i].is_in_collision = true;
					balls[j].is_in_collision = true;

					glm::vec3 dir = glm::normalize(balls[i].transform->position - balls[j].transform->position);
					balls[i].move_dir = glm::vec2(dir.y, dir.x);
					balls[i].speed = BALL_SPEED;
					balls[j].move_dir = -balls[i].move_dir;
					balls[j].speed = BALL_SPEED;
				}
			}
		}
	}
}

void PoolMode::check_collision_bb(Ball& b1) {
	if (b1.speed <= 0) return;
	if (b1.is_in_collision) return;
	for (auto& b2 : balls) {
		if (b2.is_in_collision) continue;
		float distance = glm::distance(b1.transform->position, b2.transform->position);
		if (std::abs(distance) <= b1.size.x + COLLISION_OFFSET) {
			if (b2.speed <= 0) {
				glm::vec3 dir = glm::normalize(b2.transform->position - b1.transform->position);
				b2.move_dir = glm::vec2(dir.y, dir.x);
				b2.speed = BALL_SPEED;

				b1.move_dir -= glm::dot(b2.move_dir, b1.move_dir) * b2.move_dir;
				b1.speed *= glm::length(b1.move_dir);
				b1.move_dir = glm::normalize(b1.move_dir);
			}
			else {
				glm::vec2 temp = b2.move_dir;
				b2.move_dir = b1.move_dir;
				b1.move_dir = temp;
			}
			b1.is_in_collision = true;
			b2.is_in_collision = true;
		}
	}
}

void PoolMode::check_collision_bp(Ball& ball, const Player& player) {
	float distance = glm::distance(ball.transform->position, player.transform->position);
	if (std::abs(distance) <= ball.size.x + COLLISION_OFFSET) {
		ball.speed += BALL_SPEED;
		glm::vec3 dir = glm::normalize(ball.transform->position - player.transform->position);
		ball.move_dir = glm::vec2(dir.y, dir.x);
		ball.is_in_collision = true;
	}
}

void PoolMode::check_collision_bw(Ball& ball) {
	auto& pos = ball.transform->position;
	glm::vec2 n_up = glm::vec2(0, -1);
	glm::vec2 n_down = glm::vec2(0, 1);
	glm::vec2 n_left = glm::vec2(1, 0);
	glm::vec2 n_right = glm::vec2(-1, 0);
	if (pos.x > wall[3] - ball.size.y * 0.5f) {
		pos.x = wall[3] - ball.size.y * 0.5f;
		if (ball.speed > 0)
			ball.move_dir = ball.move_dir - 2 * glm::dot(ball.move_dir, n_up) * n_up;
	}
	else if (pos.x < wall[2] + ball.size.y * 0.5f) {
		pos.x = wall[2] + ball.size.y * 0.5f;
		if (ball.speed > 0)
			ball.move_dir = ball.move_dir - 2 * glm::dot(ball.move_dir, n_down) * n_down;
	}
	else if (pos.y > wall[1] - ball.size.x * 0.5f) {
		pos.y = wall[1] - ball.size.x * 0.5f;
		if (ball.speed > 0)
			ball.move_dir = ball.move_dir - 2 * glm::dot(ball.move_dir, n_right) * n_right;
	}
	else if (pos.y < wall[0] + ball.size.x * 0.5f) {
		pos.y = wall[0] + ball.size.x * 0.5f;
		if (ball.speed > 0)
			ball.move_dir = ball.move_dir - 2 * glm::dot(ball.move_dir, n_left) * n_left;
	}
}

bool PoolMode::check_collision_bg(Ball& ball) {
	for (auto& g : goals) {
		float distance = glm::distance(ball.transform->position, g.transform->position);
		if (distance <= g.size.x * 0.5f + ball.size.x * 0.5f) {
			return true;
		}
	}
	return false;
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
		lines.draw_text("Next target: " + next_target,
		//lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Next target: " + next_target,
		//lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
