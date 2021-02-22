#ifndef PISVORK_LINKER_HPP
#define PISVORK_LINKER_HPP

#include <array>
#include <stack>
#include <vector>
#include <string>
#include <optional>

enum class Case {
	FREE,
	ALLY_STONE,
	ENEMY_STONE,
};

struct Move {
	Case type;
	unsigned x;
	unsigned y;
};

struct Threat {
	Threat(int pos_x, int pos_y, int alignment, int _isOpen);
	Threat();
	int x;
	int y;
	int size;
	bool isOpen;
};

class PiskvorkLinker {
public:
	PiskvorkLinker();
	~PiskvorkLinker();
private:
	std::array<std::array<Case, 20>, 20> m_board;
	std::vector<Threat> E_threats;
	std::vector<Threat> A_threats;
	std::stack<Move> m_history;
	std::vector<std::array<int, 2>> A_history;
	std::vector<std::array<int, 2>> E_history;
	void placeStone(unsigned x, unsigned y, Case type);
    void handleTurn(std::string data = "");
    void readBoard();
public:
	void playTurn();
	void cleanBoard();
    bool nextTurn();

	void FindThreats(unsigned x, unsigned y, Case type, std::vector<Threat> &threats);
	void get_h_Threats(int x, int y, Case type, std::vector<Threat> &threats);
	void get_v_Threats(int x, int y, Case type, std::vector<Threat> &threats);
	void get_RightDiagonal_Threats(int x, int y, Case type, std::vector<Threat> &threats);
	void get_LeftDiagonal_Threats(int x, int y, Case type, std::vector<Threat> &threats);
	std::optional<std::array<int, 2>> findBiggestThreat();
	std::array<int, 2> randomMove();

    void dumpBoard();
};

void debug_threat(std::vector<Threat> E_threats, std::vector<Threat> A_threats);
void print_threat(std::optional<Threat> threat, std::string text);
#endif
