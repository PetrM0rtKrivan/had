/*====================================
*	File name	  : win.cpp
*	Create Date	  : 09-09-2020
*	Last Modified : Tue 06 Oct 2020 07:35:23 AM CEST
*	Comment		  :
*	Created 	  : 
*=====================================*/
#include <ncurses.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <list>
#include <chrono>
#include <random>
#include <string>

using Op = bool (*)(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop);

bool up(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop);
bool down(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop);
bool left(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop);
bool right(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop);

void add_foods();
void gen_food(int w, int h);
void info_win(int w, int h, int x, int y, const std::string &, bool show_s);
bool check_food(int w, int h, int x, int y, int rx, int ry);
void update_score();
void crash_msg(const std::string &txt);
void points_up(int &rx, int &ry);
void update_speed();

struct XY {
	int x;
	int y;
	bool add;
	XY():x(0),y(0),add(false) {}
	XY(int _x, int _y, bool _add = false):x(_x), y(_y),add(_add) { }
	bool equal(int _x, int _y) const { return (x == _x) && (y == _y); }
	void cmp_mv(int &rx, int &ry) {
		if (rx < x)
			rx++;
		else if (rx > x)
			rx--;
		if (ry < y)
			ry++;
		else if (ry > y)
			ry--;
	}
};

std::list<XY> 	points;
std::list<XY> 	food;

WINDOW 	*wa = nullptr;
WINDOW 	*wm = nullptr;
WINDOW  *dbg= nullptr;
WINDOW  *info = nullptr;

#define FOOD 'm'
#define BODY 'O'

constexpr uint16_t MAX_SPEED = 100;
constexpr uint16_t MIN_SPEED = 300;
constexpr uint8_t  SPEED_RED = 20;

int mslim = MAX_SPEED;
int spd_level = 11;

int eaten = 0;
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator (seed);


struct Settings {
	public:
		int h, w, xbeg, ybeg;
		int mvx, mvy, rx, ry;
		int length;
		Settings() {
			init();
		}
		Settings(char **argv) {
			init();
			h = atoi(argv[1]), w = atoi(argv[2]), xbeg = atoi(argv[3]), ybeg = atoi(argv[4]);
		}
		void Reset() {
			mvx = mvy = rx = ry = 0;
			rx = -1;
			length = 1;
			mvx = length - 1;
		}
	private:
		void init() {
			h = w = xbeg = ybeg = 0;
			mvx = mvy = rx = ry = 0;
			rx = -1;
			length = 1;
			mvx = length - 1;
		}
};

int main(int argc, char **argv) {
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	Settings cfg(argv);
	
	int ch = 0, che = 0, cntms = 0, kpresslim = 10;
	wa = newwin(cfg.h, cfg.w, cfg.ybeg, cfg.xbeg);
	wm = newwin(cfg.h - 2, cfg.w - 2, cfg.ybeg + 1, cfg.xbeg + 1);
	dbg = newwin(5, cfg.w, cfg.ybeg, cfg.xbeg + cfg.w + 2);
	info_win(7, cfg.w, cfg.ybeg, cfg.xbeg, "Info", true);
	keypad(wm, true);
	nodelay(wm, true);
	wtimeout(wm, 0);
	chtype a = 'x', b = 'x';
	bool stop = false, exit  = false;
	box(wa, a, b);
	box(dbg, a, b);
	mvwaddstr(dbg, 1, 2, "X:");
	mvwaddstr(dbg, 2, 2, "Y:");
	mvwaddstr(dbg, 1, 11, "RX:");
	mvwaddstr(dbg, 2, 11, "RY:");
	mvwaddstr(dbg, 3, 30, "Food X:      Y:");
	mvwaddstr(dbg, 2, 50, "Sezrano!");
	mvwaddch(wm, cfg.mvy, cfg.mvx, BODY);
	gen_food(cfg.w - 3, cfg.h - 3);
	
	wrefresh(dbg);
	wrefresh(wa);
	wrefresh(wm);
	Op op = &right;
	Op prev = nullptr;
	bool p = false, quit = true;
	while(!exit) {
		ch = wgetch(wm);
		switch(ch) {
			case '+':
				if (mslim != MAX_SPEED) {
					mslim -= SPEED_RED;
					spd_level++;
				}
				break;
			case '-':
				if (mslim != MIN_SPEED) {
					mslim += SPEED_RED;
					spd_level--;
				}
				break;
			case 's':
				quit = false;
				info_win(7, cfg.w, cfg.ybeg, cfg.xbeg, "Info", false);
				break;
			case 'q':
				exit = true;
				break;
		}
		update_speed();
		//while ((che = wgetch(wm)) != ERR)
                napms(100);
		while(!quit) {
			ch = wgetch(wm);
			prev = op;
			switch(ch) {
				case KEY_LEFT:
					if (op == &right) {
						stop = true;
						crash_msg("Naboural si do sebe!");
						continue;
					}
					op   = &left;
					break;
				case KEY_RIGHT:
					if (op == &left) {
						crash_msg("Naboural si do sebe!");
						stop = true;
						continue;
					}
					op = &right;
					break;
				case KEY_UP:
					if (op == &down) {
						crash_msg("Naboural si do sebe!");
						stop = true;
						continue;
					}
					op = &up;
					break;
				case KEY_DOWN:
					if (op == &up) {
						crash_msg("Naboural si do sebe!");
						stop = true;
						continue;
					}
					op = &down;
					break;
				case KEY_HOME:
					p = !p;
					break;
				case 'r':
					wclear(wm);
					wclear(info);
					info_win(7, cfg.w, cfg.ybeg, cfg.xbeg, "Info", true);
					eaten = 0;
					food.clear();
					points.clear();
					cfg.Reset();
					gen_food(cfg.w - 3, cfg.h - 3);
					update_score();
					op = &right;
					prev = nullptr;
					p = false;
					stop = false;
					quit = true;
					continue;
				case 'q':
					quit = true;
					p = true;
					exit = true;
					break;
			}
			if (p) {
				usleep(200000);
				continue;
			}
			if (stop)
				continue;
                        if (mslim == cntms || prev != op) {
                            bool r = (*op)(cfg.mvx, cfg.mvy, cfg.rx, cfg.ry, cfg.w - 3, cfg.h - 3, stop, cfg.length, (void *)prev);
                            while ((che = wgetch(wm)) != ERR);
                            if (stop) {
                                    crash_msg("Naboural si do steny!");
                            }
                            if (r) {
                                    cfg.length++;
                                    wrefresh(dbg);
                            }
                            else
                                    mvwaddch(wm, cfg.ry, cfg.rx, ' ');
                            if (mvwinch(wm, cfg.mvy, cfg.mvx) == BODY) {
                                    stop = true;
                                    crash_msg("Pokusil ses snist sam sebe!");
                                    continue;
                            }
                            mvwaddch(wm, cfg.mvy, cfg.mvx, BODY);
                            mvwaddstr(dbg, 1, 4, std::to_string(cfg.mvx).c_str());
                            mvwaddstr(dbg, 2, 4, std::to_string(cfg.mvy).c_str());
                            mvwaddstr(dbg, 1, 14, std::to_string(cfg.rx).c_str());
                            mvwaddstr(dbg, 2, 14, std::to_string(cfg.ry).c_str());
                            wrefresh(wm);
                            wrefresh(dbg);
                            cntms = 0;
                        }
                        cntms += kpresslim;
                        napms(kpresslim);
		}
	}
	endwin();
}

bool up(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop) {
	if (prevop != &up) {
		points.push_back({x, y});
	}
	y--;
	if (!points.empty()) 
		points_up(rx, ry);
	else {
		if (ry > (y - length))
			ry--;
	}
	if (y >= 0) {
		return check_food(limx, limy, x, y, rx, ry);
	}
	else
		crash = true;
	return false;
}

bool down(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop) {
	if (prevop != &down) {
		points.push_back({x, y});
	}
	y++;
	if (!points.empty()) 
		points_up(rx, ry);
	else {
		if (ry < (y - length))
			ry++;
	}
	if (y <= limy) {
		return check_food(limx, limy, x, y, rx, ry);
	}
	else
		crash = true;
	return false;
}

bool left(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop) {
	if (prevop != &left) {
		points.push_back({x, y});
	}
	x--;
	if (!points.empty()) 
		points_up(rx, ry);
	else {
		if (rx > (x - length))
			rx--;
	}
	if (x > -1 && x < limx) {
		return check_food(limx, limy, x, y, rx, ry);
		//ry = y;
	}
	else
		crash = true;
	return false;
}

bool right(int &x, int &y, int &rx, int &ry, int limx, int limy, bool &crash, int length, void *prevop) {
	if (prevop != &right) {
		points.push_back({x, y});
	}
	x++;
	if (!points.empty()) 
		points_up(rx, ry);
	else {
		if (rx < (x - length))
			rx++;
	}
	if (x <= limx) {
		return check_food(limx, limy, x, y, rx, ry);
		//ry = y;
	}
	else
		crash = true;
	return false;
}

void gen_food(int w, int h) {
	do {
		static std::uniform_int_distribution<int> dX(0,w);
		static std::uniform_int_distribution<int> dY(0,h);
		int x = -1, y = -1;
		for (auto &e: food) {
			x = dX(generator), y = dY(generator);
			if (e.x == x && e.y == y)
				continue;
		}
		if (x == -1 && y == -1)
			x = dX(generator), y = dY(generator);
		chtype ch = mvwinch(wm, y, x);
		if (ch == BODY || ch == FOOD)
			continue;
		else {
			food.push_back({x, y, true});
			break;
		}
	}
	while(true);
	mvwaddstr(dbg, 3, 38, "   ");
	mvwaddstr(dbg, 3, 46, "   ");
	wrefresh(dbg);
	mvwaddstr(dbg, 3, 38, std::to_string(food.back().x).c_str());
	mvwaddstr(dbg, 3, 46, std::to_string(food.back().y).c_str());
	wrefresh(dbg);
	mvwaddch(wm, food.back().y, food.back().x, FOOD);
}

bool check_food(int w, int h, int x, int y, int rx, int ry) {
	bool tmp = false;
	if (!food.empty()) {
		if (food.front().x == x && food.front().y == y) {
			eaten++;
			//mvwaddstr(dbg, 2, 59, std::to_string(eaten).c_str());
			//mvwaddstr(dbg, 2, 80, "    ");
			update_score();
			gen_food(w, h);
			food.front().add = false;
			points.push_front({rx, ry});
			return true;
		}
		else {
			for (auto it = food.begin(); it != food.end();it++) {
				if (!(*it).add) {
					it = food.erase(it);
					mvwaddstr(dbg, 2, 80, "POP!");
				}
			}
		}
	}
	return tmp;
}

void info_win(int h, int w, int x, int y, const std::string &title, bool show_s) {
	info = newwin(h, w, y - h - 1, x);
	mvwaddstr(info, 0, 5, title.c_str());
	mvwaddstr(info, 1, 1, "Snezeno mysek: ");
	mvwaddstr(info, 2, 1, "Rychlost:");
	update_speed();
	if (show_s) {
		mvwaddstr(info, 4, 1, "+/- rychleji/pomaleji");
		mvwaddstr(info, 5, 1, "s - start, q - konec");
	}
	else
		mvwaddstr(info, 4, 1, "r - novy start, q - konec");
	box(info, '|', '-');
	update_score();
}

void update_speed() {
	mvwaddstr(info, 2, 11, "  ");
	mvwaddstr(info, 2, 11, std::to_string(spd_level).c_str());
	wrefresh(info);
}

void update_score() {
	mvwaddstr(info, 1, 16, "   ");
	mvwaddstr(info, 1, 16, std::to_string(eaten).c_str());
	wrefresh(info);
}

void crash_msg(const std::string &txt) {
	mvwaddstr(info, 3, 1, txt.c_str());
	wrefresh(info);
}

void points_up(int &rx, int &ry) {
	points.front().cmp_mv(rx, ry);
	if (points.front().equal(rx, ry))
		points.pop_front();
}
