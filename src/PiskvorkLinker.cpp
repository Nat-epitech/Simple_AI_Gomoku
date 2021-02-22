#include "PiskvorkLinker.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <array>
#include <unordered_map>

PiskvorkLinker::PiskvorkLinker() : m_board(), m_history()
{
    for (auto row : m_board)
        row.fill(Case::FREE);

    std::string buf;

    std::getline(std::cin, buf);
    if (buf != "START 20") {
        std::cerr << buf << std::endl;
        std::cout << "ERROR this brain can only work with 20x20 boards" << std::endl;
        throw std::runtime_error("Couldn't initialize the board");
    }
    std::cout << "OK" << std::endl;
}

PiskvorkLinker::~PiskvorkLinker()
{}

static std::array<int, 2> parseTurnInfos(const std::string& infos)
{
    auto comma = infos.find(',');

    if (comma == std::string::npos)
        throw std::invalid_argument("Invalid infos after TURN command");
    int x = std::stoi(infos.substr(0, comma));
    int y = std::stoi(infos.substr(comma + 1, infos.size() - comma - 1));
    return {x, y};
}

void PiskvorkLinker::handleTurn(std::string data)
{
    std::stringstream buf_s(data);

    buf_s >> data;
    if (data == "TURN" && buf_s.good()) {
        buf_s >> data;
        auto [x, y] = parseTurnInfos(data);
        placeStone(x, y, Case::ENEMY_STONE);
        E_history.push_back(std::array<int, 2>{x, y});
    }
}

void PiskvorkLinker::readBoard()
{
    std::string buf;

    std::getline(std::cin, buf);
    if (buf == "DONE")
        return;
    auto [x, y] = parseTurnInfos(buf.substr(0, buf.size() - 2));
    Case stone = (buf.back() == '1') ? (Case::ALLY_STONE) : (Case::ENEMY_STONE);
    (buf.back() == '1') ? A_history.push_back(std::array<int, 2>{x, y}) : E_history.push_back(std::array<int, 2>{x, y});
    placeStone(x, y, stone);
    readBoard();
}

void PiskvorkLinker::placeStone(unsigned x, unsigned y, Case type)
{
    if (m_board.at(x).at(y) != Case::FREE)
        throw std::logic_error("Placing stone on a non-free case");
    m_board.at(x).at(y) = type;
    //dumpBoard();
}

void PiskvorkLinker::playTurn()
{
    if (!A_history.empty()){
        for (long unsigned int i = 0; i < A_history.size(); i++)
            FindThreats(A_history[i][0], A_history[i][1], Case::ALLY_STONE, A_threats);
    }
    if (!E_history.empty()){
        for (long unsigned int i = 0; i < E_history.size(); i++)
            FindThreats(E_history[i][0], E_history[i][1], Case::ENEMY_STONE, E_threats);
    }
    //debug_threat(E_threats, A_threats);
    std::optional<std::array<int, 2>> next_move = findBiggestThreat();
    placeStone(next_move->at(0), next_move->at(1), Case::ALLY_STONE);
    std::cout << next_move->at(0) << "," << next_move->at(1) << std::endl;
    A_history.push_back(*next_move);
    A_threats.clear();
    E_threats.clear();
}

bool PiskvorkLinker::nextTurn()
{
    std::string buf;

    std::getline(std::cin, buf);
    if (buf == "BEGIN")
        return true;
    if (buf.starts_with("TURN"))
        handleTurn(buf);
    else if (buf == "END")
        return false;
    else if (buf == "BOARD") {
        readBoard();
        return true;
    } else if (buf == "ABOUT") {
        std::cout << "name=\"GomokuAI\", version=\"1.0\", author=\"Naelriun, Natou74 et Gerox\", country=\"USA\"" << std::endl;
        return nextTurn();
    } else if (buf.starts_with("INFO")) {
        std::cout << "MESSAGE " << buf << std::endl;
        return nextTurn();
    } else {
        std::cout << "UNKNOWN command" << std::endl;
        return nextTurn();
    }
    return true;
}

void PiskvorkLinker::cleanBoard()
{
    for (auto row : m_board)
        row.fill(Case::FREE);
    A_threats.clear();
    E_threats.clear();
    A_history.clear();
    E_history.clear();
}

std::array<int, 2> PiskvorkLinker::randomMove()
{
    for (int j = 0; j < 9; j++){
        for (int i = 0; i <= 9; i++)
        {
            if (m_board.at(9 - j).at(9 - i) == Case::FREE)
                return {9 - j, 9 - i};
            if (m_board.at(9 - j).at(9 + i) == Case::FREE)
                return {9 - j, 9 + i};
            if (m_board.at(9 + j).at(9 + i) == Case::FREE)
                return {9 + j, 9 + i};
            if (m_board.at(9 + j).at(9 - i) == Case::FREE)
                return {9 + j, 9 - i};
        }
    }
    for (int i = 0; i <= 19; i++){
        if (m_board.at(19).at(i) == Case::FREE)
            return {19, i};
    }
    for (int i = 0; i <= 19; i++){
        if (m_board.at(i).at(19) == Case::FREE)
            return {i, 19};
    }
    return {};
}

std::optional<Threat> findBiggestThreatInList(std::vector<Threat> threats)
{
    Threat save;

    if (threats.empty())
        return {};
    save = threats.at(0);
    for (long unsigned int i = 1; i < threats.size(); i++){
        if (save.size != 5 && ((threats.at(i).isOpen && !save.isOpen) || threats.at(i).size == 5 || (threats.at(i).isOpen == save.isOpen && save.size < threats.at(i).size)))
            save = threats.at(i);
    }
    return (save);
}

std::optional<std::array<int, 2>> PiskvorkLinker::findBiggestThreat()
{
    auto enemy_case = findBiggestThreatInList(E_threats);
    auto ally_case = findBiggestThreatInList(A_threats);

    if (!ally_case && !enemy_case)
        return randomMove();
    //print_threat(enemy_case, "Enemy : ");
    //print_threat(ally_case, "Allié : ");
    if (!ally_case)
        return {{enemy_case->x, enemy_case->y}};
    if (!enemy_case)
        return {{ally_case->x, ally_case->y}};
    if (ally_case->size != 5 && enemy_case->size >= 3 && ((enemy_case->isOpen && !ally_case->isOpen) || enemy_case->size == 5 || (enemy_case->isOpen == ally_case->isOpen && ally_case->size < enemy_case->size)))
        return {{enemy_case->x, enemy_case->y}};
    return {{ally_case->x, ally_case->y}};
}

void PiskvorkLinker::get_h_Threats(int x, int y, Case type, std::vector<Threat> &threats)
{
    int f_tmp = 0, s_tmp = 0;
    int save_x = x;
    int f_pos_x, s_pos_x;

    for (; x >= 0 && m_board.at(x).at(y) == type; x--);
    if (x >= 0 && m_board.at(x).at(y) == Case::FREE){
        f_pos_x = x;
        for (x = f_pos_x - 1; x >= 0 && m_board.at(x).at(y) == type; x--)
            f_tmp++;
        for (x = f_pos_x + 1; x <= 19 && m_board.at(x).at(y) == type; x++)
            f_tmp++;
        f_tmp++;
    }
    for (x = save_x; x <= 19 && m_board.at(x).at(y) == type; x++);
    if (x <= 19 && m_board.at(x).at(y) == Case::FREE){
        s_pos_x = x;
        for (x = s_pos_x - 1; x >= 0 && m_board.at(x).at(y) == type; x--)
            s_tmp++;
        for (x = s_pos_x + 1; x <= 19 && m_board.at(x).at(y) == type; x++)
            s_tmp++;
        s_tmp++;
    }
    if (f_tmp == 0 && s_tmp == 0)
         return;
    (f_tmp > s_tmp) ? threats.push_back(Threat(f_pos_x, y, f_tmp, f_tmp * s_tmp)) : threats.push_back(Threat(s_pos_x, y, s_tmp, f_tmp * s_tmp));
}

void PiskvorkLinker::get_v_Threats(int x, int y, Case type, std::vector<Threat> &threats)
{
    int f_tmp = 0, s_tmp = 0;
    int save_y = y;
    int f_pos_y, s_pos_y;

    for (; y >= 0 && m_board.at(x).at(y) == type; y--);
    if (y >= 0 && m_board.at(x).at(y) == Case::FREE){
        f_pos_y = y;
        for (y = f_pos_y - 1; y >= 0 && m_board.at(x).at(y) == type; y--)
            f_tmp++;
        for (y = f_pos_y + 1; y <= 19 && m_board.at(x).at(y) == type; y++)
            f_tmp++;
        f_tmp++;
    }
    for (y = save_y; y <= 19 && m_board.at(x).at(y) == type; y++);
    if (y <= 19 && m_board.at(x).at(y) == Case::FREE){
        s_pos_y = y;
        for (y = s_pos_y - 1; y >= 0 && m_board.at(x).at(y) == type; y--)
            s_tmp++;
        for (y = s_pos_y + 1; y <= 19 && m_board.at(x).at(y) == type; y++)
            s_tmp++;
        s_tmp++;
    }
    if (f_tmp == 0 && s_tmp == 0)
        return;
    (f_tmp > s_tmp) ? threats.push_back(Threat(x, f_pos_y, f_tmp, f_tmp * s_tmp)) : threats.push_back(Threat(x, s_pos_y, s_tmp, f_tmp * s_tmp));
}

void PiskvorkLinker::get_RightDiagonal_Threats(int x, int y, Case type, std::vector<Threat> &threats)
{
    int f_tmp = 0, s_tmp = 0;
    int save_x = x, save_y = y;
    int f_pos_x, s_pos_x, f_pos_y, s_pos_y;

    for (; x >= 0 && y <= 19 && m_board.at(x).at(y) == type; x--, y++);
    if (x >= 0 && y <= 19 && m_board.at(x).at(y) == Case::FREE){
        f_pos_x = x;
        f_pos_y = y;
        for (x = f_pos_x - 1, y = f_pos_y + 1; x >= 0 && y <= 19 && m_board.at(x).at(y) == type; x--, y++)
            f_tmp++;
        for (x = f_pos_x + 1, y = f_pos_y - 1; x <= 19 && y >= 0 && m_board.at(x).at(y) == type; x++, y--)
            f_tmp++;
        f_tmp++;
    }
    for (y = save_y, x = save_x; y >= 0 && x <= 19 && m_board.at(x).at(y) == type; x++, y--);
    if (y >= 0 && x <= 19 && m_board.at(x).at(y) == Case::FREE){
        s_pos_x = x;
        s_pos_y = y;
        for (x = s_pos_x - 1, y = s_pos_y + 1; x >= 0 && y <= 19 && m_board.at(x).at(y) == type; x--, y++)
            s_tmp++;
        for (x = s_pos_x + 1, y = s_pos_y - 1; x <= 19 && y >= 0 && m_board.at(x).at(y) == type; x++, y--)
            s_tmp++;
        s_tmp++;
    }
    if (f_tmp == 0 && s_tmp == 0)
        return;
    (f_tmp > s_tmp) ? threats.push_back(Threat(f_pos_x, f_pos_y, f_tmp, f_tmp * s_tmp)) : threats.push_back(Threat(s_pos_x, s_pos_y, s_tmp, f_tmp * s_tmp));
}

void PiskvorkLinker::get_LeftDiagonal_Threats(int x, int y, Case type, std::vector<Threat> &threats)
{
    int f_tmp = 0, s_tmp = 0;
    int save_x = x, save_y = y;
    int f_pos_x, s_pos_x, f_pos_y, s_pos_y;

    for (; x >= 0 && y >= 0 && m_board.at(x).at(y) == type; x--, y--);
    if (x >= 0 && y >= 0 && m_board.at(x).at(y) == Case::FREE){
        f_pos_x = x;
        f_pos_y = y;
        for (x = f_pos_x - 1, y = f_pos_y - 1; x >= 0 && y >= 0 && m_board.at(x).at(y) == type; x--, y--)
            f_tmp++;
        for (x = f_pos_x + 1, y = f_pos_y + 1; x <= 19 && y <= 19 && m_board.at(x).at(y) == type; x++, y++)
            f_tmp++;
        f_tmp++;
    }
    for (y = save_y, x = save_x; y <= 19 && x <= 19 && m_board.at(x).at(y) == type; x++, y++);
    if (y <= 19 && x <= 19 && m_board.at(x).at(y) == Case::FREE){
        s_pos_x = x;
        s_pos_y = y;
        for (x = s_pos_x - 1, y = s_pos_y - 1; x >= 0 && y >= 0 && m_board.at(x).at(y) == type; x--, y--)
            s_tmp++;
        for (x = s_pos_x + 1, y = s_pos_y + 1; x <= 19 && y <= 19 && m_board.at(x).at(y) == type; x++, y++)
            s_tmp++;
        s_tmp++;
    }
    if (f_tmp == 0 && s_tmp == 0)
        return;
    (f_tmp > s_tmp) ? threats.push_back(Threat(f_pos_x, f_pos_y, f_tmp, f_tmp * s_tmp)) : threats.push_back(Threat(s_pos_x, s_pos_y, s_tmp, f_tmp * s_tmp));
}

void PiskvorkLinker::FindThreats(unsigned x, unsigned y, Case type, std::vector<Threat> &threats)
{
    get_h_Threats(x, y, type, threats);
    get_v_Threats(x, y, type, threats);
    get_LeftDiagonal_Threats(x, y, type, threats);
    get_RightDiagonal_Threats(x, y, type, threats);
}

// Threat Implementations

Threat::Threat(int pos_x, int pos_y, int alignment, int _isOpen): x(pos_x), y(pos_y), size(alignment), isOpen(false)
{
    if (_isOpen != 0)
        isOpen = true;
}

Threat::Threat(){}

// Debug fonctions

void PiskvorkLinker::dumpBoard()
{
    std::unordered_map<Case, char> caseChars = {
    {Case::FREE, '_'},
    {Case::ALLY_STONE, 'X'},
    {Case::ENEMY_STONE, 'O'}};

    std::cout << "     [BOARD]     " << std::endl;
    for (auto row : m_board) {
        for (auto boardCase : row)
            std::cerr << caseChars[boardCase];
        std::cerr << std::endl;
    }
    std::cout << std::endl;
}

void print_threat(std::optional<Threat> tmp, std::string text)
{
    std::cout << text << "x = " << tmp->x << " y = " << tmp->y << " menace de niveau = " << tmp->size;
    if (tmp->isOpen)
            std::cout << " (ouverte)" << std::endl;
        else
            std::cout << " (fermée)" << std::endl;
}

void debug_threat(std::vector<Threat> E_threats, std::vector<Threat> A_threats)
{
    if (A_threats.empty())
        std::cout << "A_threat vide" << std::endl;
    else{
        std::cout << "Liste coup a jouer pour contre une menace ennemy :" << std::endl;
        for (int i = 0; i < E_threats.size() ; i++){
            std::cout << "x = " << E_threats.at(i).x << " y = " << E_threats.at(i).y << " Menace ennemy de niveau = " << E_threats.at(i).size;
            if (E_threats.at(i).isOpen)
                std::cout << " la menace est ouverte" << std::endl;
            else
                std::cout << " la menace est fermée" << std::endl;
        }
        std::cout << std::endl;
    }
    if (E_threats.empty())
        std::cout << "E_threat vide" << std::endl;
    else{
        std::cout << "Liste coup a jouer pour appuyer une menace allié :" << std::endl;
        for (int i = 0; i < A_threats.size() ; i++){
            std::cout << "x = " << A_threats.at(i).x << " y = " << A_threats.at(i).y << " Menace ennemy de niveau = " << A_threats.at(i).size;
            if (A_threats.at(i).isOpen)
                std::cout << " la menace est ouverte" << std::endl;
            else
                std::cout << " la menace est fermée" << std::endl;
        }
        std::cout << std::endl;
    }
}
