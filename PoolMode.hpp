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
	glm::vec3 size = glm::vec3(0.3f,0.3f,0.3f);
	glm::vec4 collision_box = glm::vec4(0);
};

struct Player {
	Player() {}
	Player(Scene::Transform* ts) : transform(ts) {}
	Scene::Transform* transform = nullptr;
	glm::vec3 size = glm::vec3(0.3f, 0.3f, 0.2f);
	glm::vec4 collision_box = glm::vec4(0);
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
	std::vector<Ball> balls;

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
	void update_player_movement(float elapsed);
	void update_camera(float elapsed);
	void update_player_collision(glm::vec2(movement));
};
