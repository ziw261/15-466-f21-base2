#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>


enum class Ball_Color {
	None,
	Black,
	Blue,
	Brown,
	Green,
	Orange,
	Pink,
	Red,
	Yellow,
};


struct Ball {
	Ball(Ball_Color bc, bool isf, Scene::Transform* ts)
		: ball_color(bc), is_flower(isf), transform(ts) {}

	Ball_Color ball_color = Ball_Color::None;
	bool is_flower = false;
	Scene::Transform* transform = nullptr;
	float speed = 0.0f;
	glm::vec2 move_dir = glm::vec2(0.0f);
	glm::vec3 size = glm::vec3(0.3f,0.3f,0.3f);
};

struct Player {
	Player() {}
	Player(Scene::Transform* ts) : transform(ts) {}
	Scene::Transform* transform = nullptr;
	glm::vec3 size = glm::vec3(0.3f, 0.3f, 0.2f);
};

struct Goal {
	Goal () {}
	Goal(Scene::Transform* ts) : transform(ts) {}
	Scene::Transform* transform = nullptr;
	glm::vec3 size = glm::vec3(0.8f, 0.8f, 0.02f);
};

struct PoolMode : Mode {
	PoolMode();
	virtual ~PoolMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	Player player;
	float player_speed = 1.0f;
	std::vector<Ball> balls;
	std::vector<Goal> goals;
	glm::vec4 wall = glm::vec4(0.0f);

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, cam_zoomin, cam_zoomout;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;


	void loadObjects();
	void create_walls();
	void update_player_movement(float elapsed);
	void update_ball_movement(float elapsed);
	void update_camera(float elapsed);
	bool check_collision_bb(const Ball& b1, const Ball& b2);
	bool check_collision_bp(const Ball& ball, const Player& player);
	void check_collision_bw(Ball& ball);
};
