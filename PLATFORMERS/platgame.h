#pragma once

#ifdef PLATGAME_EXPORTS
#define PLAT_API __declspec(dllexport)
#else
#define PLAT_API __declspec(dllimport)
#endif

#include <random>
#include <vector>

constexpr float scr_width{ 1000.0f };
constexpr float scr_height{ 800.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 750.0f };

constexpr char UP_FLAG = 0b00000001;
constexpr char DOWN_FLAG = 0b00000010;
constexpr char LEFT_FLAG = 0b00000100;
constexpr char RIGHT_FLAG = 0b00001000;
constexpr char JUMP_FLAG = 0b00010000;
constexpr char FALL_FLAG = 0b00100000;

enum class dirs { up = 0, down = 1, left = 2, right = 3, stop = 4 };
enum class creatures {
	evil1 = 0, evil2 = 1, evil3 = 2, evil4 = 3,
	evil5 = 4, evil6 = 5, hero = 6
};
enum class fields { field1 = 0, field2 = 1, field3 = 2, platform = 3 };

namespace gamedll
{

	class PLAT_API RANDENGINE
	{
	private:
		std::random_device rd{};
		std::vector<unsigned int> vSeeds;
		std::seed_seq* seed_seq_ptr = nullptr;
		std::mt19937* twister = nullptr;

	public:

		RANDENGINE();

		~RANDENGINE();

		int generate(int min, int max);
	};

	class PLAT_API ATOMS
	{
	protected:
		float width{};
		float height{};

	public:
		float x{ 0 };
		float y{ 0 };
		float ex{ 0 };
		float ey{ 0 };

		ATOMS(float _x = 0, float _y = 0, float _width = 1, float _height = 1);
		virtual ~ATOMS() {};

		float GetWidth() const;
		float GetHeight() const;

		void SetWidth(float _width);
		void SetHeight(float _height);

		void SetEdges();
		void NewDims(float _width, float _height);
	};

	class PLAT_API ATOM_CONTAINER
	{
	private:
		ATOMS* base_ptr{ nullptr };
		ATOMS* end_ptr{ nullptr };

		int num_of_elements{ 0 };
		int next_push_back{ 0 };

	public:

		ATOM_CONTAINER(int max_size);
		~ATOM_CONTAINER();

		void push_back(ATOMS& element);
		void push_front(ATOMS& element);

		int size() const;

		ATOMS& operator[] (int position);
		void operator()(ATOMS& element, int position);
	};

	class PLAT_API FIELD :virtual public ATOMS
	{
	protected:
		fields type{};
		unsigned char flags{ 0 };

		int frame{ 0 };
		int max_frames{ 0 };
		int frame_delay{ 0 };

	public:
		dirs dir{};
		float speed{ 0 };

		FIELD(fields what_type, float _x);
		virtual ~FIELD() {};

		void Release();
		fields GetType() const;
		int GetFrame();

		bool GetFlag(unsigned char which_flag) const;
		void SetFlag(unsigned char which_flag);
		void NullFlag(unsigned char which_flag);
		void Move(float gear);
	};

	class PLAT_API CREATURE :public ATOMS
	{
	protected:
		unsigned char flags{ 0 };

		float start_x{};
		float start_y{};
		float end_x{};
		float end_y{};

		float slope{};
		float intercept{};
		float speed{ 1.0f };

		bool vert_path = false;
		bool hor_path = false;

		int max_frames{};
		int frame{};
		int frame_delay{};

		int strenght{ 0 };
		int attack_delay{ 0 };
		int attack_cooldown{ 0 };

		creatures type{};

		void SetPathInfo(float where_x, float where_y);

	public:
		dirs dir{};
		int lifes{ 0 };

		CREATURE(creatures what, float start_x, float start_y);
		virtual ~CREATURE() {};

		virtual void Release() = 0;

		virtual bool GetFlag(unsigned char which_flag) const = 0;
		virtual void SetFlag(unsigned char which_flag) = 0;
		virtual void NullFlag(unsigned char which_flag) = 0;

		creatures GetType() const;

		int Attack();
		bool Falling(ATOM_CONTAINER& platforms);

		virtual int GetFrame() = 0;

		virtual unsigned char Move(float gear, bool change_path = false, float path_ex = 0, float path_ey = 0) = 0;
	};

	class PLAT_API EVILS :public CREATURE
	{
	public:
		EVILS(creatures what, float _x, float _y) :CREATURE(what, _x, _y) {};

		void Release();
		bool GetFlag(unsigned char which_flag) const;
		void SetFlag(unsigned char which_flag);
		void NullFlag(unsigned char which_flag);
		int GetFrame();
		unsigned char Move(float gear, bool change_path, float path_ex, float path_ey);
	};

	class PLAT_API HERO :public CREATURE
	{
	public:

		HERO(float _x, float _y) :CREATURE(creatures::hero, _x, _y) {};

		void Release();
		bool GetFlag(unsigned char which_flag) const;
		void SetFlag(unsigned char which_flag);
		void NullFlag(unsigned char which_flag);
		int GetFrame();
		unsigned char Move(float gear, bool change_path, float path_ex, float path_ey);

	};

	typedef CREATURE* Creature;

	//FACTORY **********************

	extern PLAT_API CREATURE* CreatureFactory(creatures which_creature, float start_x, float start_y);
}