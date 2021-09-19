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
	Scene::Transform* transform;
	Ball_Color ball_color = Ball_Color::None;
	bool is_flower = false;
	Ball(Ball_Color bc, bool isf, Scene::Transform* ts)
		: ball_color(bc), is_flower(isf), transform(ts) {}
};

struct Player {
	Scene::Transform* transform;

};

struct PoolMode : Mode {
	PoolMode();
	virtual ~PoolMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

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

};
