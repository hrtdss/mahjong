#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <ctime>
#include "graphics.h"

using namespace std;

const int WIDTH = 1280; // Ширина окна
const int HEIGHT = 960; // Высота окна
const int DICEWIDTH = 70; // Ширина кости
const int DICEHEIGHT = 90; // Высота кости
const int DECKSIZE = 144; // Общее кол-во карт

struct buttons {
   int left, top, w, h; // Координаты левого верхнего угла кнопки, ее ширина и высота
};

struct dice {
   int type; // Вид кости
   int rank; // Достоинство кости
   int x, y, z; // Положение на игровом столе
   bool isSelected; // Флаг для исключения возможности выбрать карту 2 раза
};

struct selectedDice {
   int id; // Номер выбранной кости из массива deck
   int x, y; // Положение выбранной кости в массиве field
};

ifstream in; // Входной файловый поток
IMAGE *mainMenu, *gameWindow, *rulesScreen, *informationScreen;
buttons bt[4]; // Массив кнопок

int initX = 114, initY = 127 ; // Откуда начинается отрисовка карт
int field[8][14]; // Массив для хранения высоту каждой ячейки поля
int fieldOfDice[4][8][14]; // Массив хранящий номер кости для каждой непустой ячейки на всех уровнях фигуры
vector <dice> deck; // Массив всех костей

int menuState; // В каком пункте меню находится пользователь
int currDeckSize; // Количество оставшихся на столе костей
int amountOfScores; // Набранные за текущую игру очки
int numberOfPossibleMoves; // Количество доступных для выбора пар костей

void loadFiles(); // Загрузка изображений
void initBasicButtons(); // Определение границ кнопок
int menuButtons(int currStatus); // Выбор пункта меню
void showSubItem(int a); // Переход в пункт с правилами/информацией
void deckProcessing(); // Создание и перемешивание колоды костей
void defineDice(int id, double a, double b, int c); // Доопределение полей dice для каждой кости из deck
void resetDice(int id); // Переопределение полей кости из deck (деактивация для текущей  игры)
void fieldsCreation(); // Подготовка ключевых переменных и массивов к новой игре
int findAvailablePairs(); // Подсчет количества доступных для удаления пар костей
void drawDice(int id); // Отрисовка кости по ее индексу в массиве deck
void drawDesk(); // Отрисовка игрового поля и интерфейса
void gameSituation(int a); // Действия в случае победы/проигрыша
void startGame(); // Процесс игры

int main() {
   srand(time(NULL));
   initwindow(WIDTH, HEIGHT, "Mahjong", 300, 40);

   loadFiles();
   initBasicButtons();

   do {
      menuState = menuButtons(menuState);

      switch (menuState) {
      case 1: {
            deckProcessing();
            fieldsCreation();
            startGame();
            break;
         }
      case 2: {
            showSubItem(1);
            break;
         }
      case 3: {
            showSubItem(2);
            break;
         }
      }
   } while (menuState != 4);

   closegraph();
   return 0;
}

void loadFiles() {
   mainMenu = loadBMP("images/menu.bmp");
   gameWindow = loadBMP("images/game_window.bmp");
   rulesScreen = loadBMP("images/rules.bmp");
   informationScreen = loadBMP("images/about.bmp");

   setbkcolor(COLOR(140, 15, 7));
   setcolor(WHITE);
   settextstyle(BOLD_FONT, HORIZ_DIR, 5);
}

void initBasicButtons() {
   // новая игра
   bt[0].left = 471;
   bt[0].top = 303;
   bt[0].w = 339;
   bt[0].h = 64;
   // правила
   bt[1].left = 517;
   bt[1].top = 479;
   bt[1].w = 244;
   bt[1].h = 64;
   // информация
   bt[2].left = 469;
   bt[2].top = 665;
   bt[2].w = 326;
   bt[2].h = 61;
   // вернуться в меню
   bt[3].left = 49;
   bt[3].top = 876;
   bt[3].w = 149;
   bt[3].h = 44;
}

int menuButtons(int currStatus) {
   putimage(0, 0, mainMenu, COPY_PUT);

   int x, y;

   while (true) {
      do {
         x = mousex();
         y = mousey();
      } while (mousebuttons() != 1);
      delay(100);

      for (int i = 0; i < 3; i++)
         if (x >= bt[i].left && y >= bt[i].top && x <= bt[i].left + bt[i].w && y <= bt[i].top + bt[i].h) { // Определении кнопки меню по положению курсора
            return i + 1;
         }
   }

   return currStatus;
}

void showSubItem(int a) {
   IMAGE *background = (a == 1) ? rulesScreen : informationScreen;
   putimage(0, 0, background, COPY_PUT);
   freeimage(background);

   int x, y;

   while (true) {
      do {
         x = mousex();
         y = mousey();
      } while (mousebuttons() != 1);
      delay(100);

      if (x >= bt[3].left && y >= bt[3].top && x <= bt[3].left + bt[3].w && y <= bt[3].top + bt[3].h) { // Кнопка возврата в меню
         return;
      }
   }
}

void deckProcessing() {
   deck.clear();
   dice d[42];

   // Создание всех 42 карт
   int h = 0;
   for (int i = 1; i <= 3; i++) {
      for (int j = 1; j <= 9; j++) {
         d[h].type = i;
         d[h].rank = j;
         h++;
      }
   }

   for (int k = 4; k <= 7; k++) {
      int l = (k == 5) ? 3 : 4;
      for (int p = 1; p <= l; p++) {
         d[h].type = k;
         d[h].rank = p;
         h++;
      }
   }

   // Создание обходимого количества копий каждой карты для формирования стандартной колоды
   for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 34; j++) { // 42 - 8 (h - 8)
         deck.push_back(d[j]);
      }
   }

   for (int j = 34; j < 42; j++) {
      deck.push_back(d[j]);
   }

   // Перемешивание колоды
   random_shuffle(deck.begin(), deck.end());
}

void defineDice(int id, double a, double b, int c) {
   deck[id].x = initX + DICEWIDTH * a;
   deck[id].y = initY + DICEHEIGHT * b;
   deck[id].z = c;
   deck[id].isSelected = false;
}

void resetDice(int id) {
   deck[id].type = 0;
   deck[id].rank = 0;
   deck[id].x = -1;
   deck[id].y = -1;
   deck[id].z = -1;
}

void fieldsCreation() {
   currDeckSize = DECKSIZE;
   amountOfScores = 0;
   int temp[8][14]; // Вспомогательный массив для заполнения fieldOfDice

   in.open("field(1).txt");
   int a;
   for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 14; j++) {
         in >> a;
         field[i][j] = a;
         temp[i][j] = a;
      }
   }
   in.close();

   int h = 0;
   for (int z = 0; z < 4; z++) {
      for (int y = 0; y < 8; y++) {
         for (int x = 0; x < 14; x++) {
            if (temp[y][x] == 4 - z) {
               fieldOfDice[3 - z][y][x] = h;

               defineDice(h, x, y, 4 - z);
               h++;

               temp[y][x]--;
            }
            else {
               fieldOfDice[3 - z][y][x] = -1;
            }
         }
      }
   }

   double b[4] = {0, 6.5, 13, 14};
   int c[4] = {1, 5, 1, 1};
   for (int i = 0; i < 4; i++) {
      defineDice(h, b[i], 3.5, c[i]);
      h++;
   }
}

int findAvailablePairs() {
   vector <int> check;

   for (int i = 140; i < DECKSIZE; i++) {
      if (i == 142 && deck[i + 1].rank != 0)
         continue;
      else if (deck[i].rank != 0){
         check.push_back(i);
      }
   }
   
   int id;
   for (int g = 0; g < 8; g++) {
      for (int h = 0; h < 13; h++) {
         id = fieldOfDice[field[g][h] - 1][g][h];
         if (((id == 86 || id == 98) && deck[140].rank != 0) || ((id == 97 || id == 109) && deck[142].rank != 0)) {
            continue;
         }
         else if ((id >= 0 && id <= 3 && deck[141].rank == 0) || id >= 4) { 
            if (fieldOfDice[field[g][h] - 1][g][h + 1] == -1 || fieldOfDice[field[g][h] - 1][g][h - 1] == -1) {
               check.push_back(id);
            }
         }
      }
   }
   
   bool visited[check.size()];
   fill(visited, visited + check.size(), false);
//cout << "===================================" << endl;
   int counter = 0;
   for (int u = 0; u < check.size(); u++) {
      for (int i = u + 1; i < check.size(); i++) {
         if (!visited[u] && !visited[i] && deck[check[u]].type == deck[check[i]].type) {
            if ((deck[check[u]].type <= 5 && deck[check[u]].rank == deck[check[i]].rank) || deck[check[u]].type >= 6) {
               visited[u] = true;
               visited[i] = true;
               //cout << check[u] << ' ' << check[i] << endl;
               counter++;
            }
         }
      }
   }
//cout << "===================================" << endl;
   return counter;
}

void drawDice(int id) {
   char dice_name[20];
   snprintf(dice_name, 20, "%s%d%s%d%s", "images/dice/", deck[id].type, "-", deck[id].rank, ".bmp");

   IMAGE *t_dice = loadBMP(dice_name);
   putimage(deck[id].x, deck[id].y, t_dice, COPY_PUT);
   freeimage(t_dice);
}

void drawDesk() {
   setactivepage(1);
   setvisualpage(0);
   putimage(0, 0, gameWindow, COPY_PUT);

   //~ cout << "***********************************" << endl;
   //~ for (int x = 0; x < 8; x++) {
   //~ for (int y = 0; y < 14; y++) {
   //~ cout << field[x][y] << ' ';
   //~ }
   //~ cout << endl;
   //~ }
   //~ cout << "***********************************" << endl;

   ///////////////////////
   //~ for (int i = 0; i < 8; i++) {
   //~ for (int j = 0; j < 14; j++) {
   //~ if (field[i][j] > 0)
   //~ cout << setw(5) << fieldOfDice[field[i][j] - 1][i][j] <<  ' '; // fieldOfDice[field[i][j] - 1][i][j]
   //~ else
   //~ cout << setw(5) << "-1" << ' ';
   //~ }
   //~ cout << endl;
   //~ }
   //~ cout << endl;
   //~ for (int i = 0; i < 8; i++) {
   //~ for (int j = 0; j < 14; j++) {
   //~ if (field[i][j] > 0)
   //~ cout << deck[fieldOfDice[field[i][j] - 1][i][j]].type << "-" << deck[fieldOfDice[field[i][j] - 1][i][j]].rank <<  ' '; // fieldOfDice[field[i][j] - 1][i][j]
   //~ else
   //~ cout << "   " << ' ';
   //~ }
   //~ cout << endl;
   //~ }
   ///////////////////////

   for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 14; j++) {
         if (field[i][j] > 0) {
            drawDice(fieldOfDice[field[i][j] - 1][i][j]);
         }
      }
   }

   for (int i = 140; i < DECKSIZE; i++) {
      if (deck[i].z > 0) {
         drawDice(i);
      }
   }

   int a[3] = {410, 674, 1011}, b[3] = {numberOfPossibleMoves, amountOfScores, currDeckSize};
   for (int i = 0; i < 3; i++) {
      string s = to_string(b[i]);
      outtextxy(a[i], 26, s.c_str());
   }

   setvisualpage(1);
}

void gameSituation(int a) {
   IMAGE *res = (a == 1) ? loadBMP("images/win.bmp") : loadBMP("images/lose.bmp");
   putimage(0, 0, res, COPY_PUT);

   getch();
   free(res);
}

void startGame() {
   putimage(0, 0, gameWindow, COPY_PUT);

   int x, y;
   int k, j; // Положение выбранной кости в массиве field
   int top_id = 141; // Номер самое верхней кости
   bool isRenderingDone = false; // Флаг для исключения повторной отрисовки игрового поля интерфейса без необходимости
   vector <selectedDice> ij; // Массив для хранения выбранных костей

   while (currDeckSize != 0) {
      numberOfPossibleMoves = findAvailablePairs();
      if (numberOfPossibleMoves == 0) {
         gameSituation(2);
      }

      if (!isRenderingDone) {
         drawDesk();
         isRenderingDone = true;
      }

      do {
         x = mousex();
         y = mousey();
      } while (mousebuttons() != 1);
      
      delay(400);
      
      if (x >= bt[3].left && y >= bt[3].top && x <= bt[3].left + bt[3].w && y <= bt[3].top + bt[3].h) { // Кнопка возврата в меню
         return;
      }

      if (ij.size() != 2) { // Выбор пары костей для проверки на возможность удаления
         for (int i = 140; i < DECKSIZE; i++) {
            if ((x >= deck[i].x && y >= deck[i].y && x <= deck[i].x + DICEWIDTH && y <= deck[i].y + DICEHEIGHT) && !deck[i].isSelected) {
               if (i == 142 && deck[i + 1].rank != 0)
                  continue;
               else {
                  deck[i].isSelected = true;
                  ij.push_back({i, -1, -1});
               }
            }
         }

         for (int i = 0; i < DECKSIZE - 4; i++) {
            if ((x >= deck[i].x && y >= deck[i].y && x <= deck[i].x + DICEWIDTH && y <= deck[i].y + DICEHEIGHT)) {
               k = (x - initX) / DICEWIDTH;
               j = (y - initY) / DICEHEIGHT;

               if (!deck[i].isSelected && deck[i].z == field[j][k] && (fieldOfDice[deck[i].z - 1][j][k + 1] == -1 || fieldOfDice[deck[i].z - 1][j][k - 1] == -1)) { // нужна проверка на выход за границы массива
                  if ((i >= 0 && i <= 3 && deck[top_id].rank == 0) || i >= 4) {
                     if (((i == 86 || i == 98) && deck[140].rank != 0) || ((i == 97 || i == 109) && deck[142].rank != 0))
                        continue;
                     else {
                        deck[i].isSelected = true;
                        ij.push_back({i, j, k});
                     }
                  }
               }
            }
         }
      }

      if (ij.size() == 2) {
         //cout << deck[ij[0].id].type << "-" << deck[ij[0].id].rank << "_______" << deck[ij[1].id].type << "-" << deck[ij[1].id].rank << endl;
         if (deck[ij[0].id].type == deck[ij[1].id].type) {
            if ((deck[ij[0].id].type <= 5 && deck[ij[0].id].rank == deck[ij[1].id].rank) || deck[ij[0].id].type >= 6) {
               resetDice(ij[0].id);
               resetDice(ij[1].id);
            }
         }

         if (deck[ij[0].id].type == 0 && deck[ij[1].id].type == 0) {
            for (int i = 0; i < 2; i++) {
               if (ij[i].x != -1) {
                  fieldOfDice[field[ij[i].x][ij[i].y] - 1][ij[i].x][ij[i].y] = -1;
                  field[ij[i].x][ij[i].y]--;
               }
            }

            isRenderingDone = false;
            amountOfScores += 15;
            currDeckSize -= 2;
         }

         for (int i = 0; i < 2; i++) {
            deck[ij[i].id].isSelected = false;
         }

         ij.clear();
      }

      if (currDeckSize == 0) {
         gameSituation(1);
      }
   }
}