#include <stdlib.h> 

int right = 1;
int left = -1;
int up = 2;
int down = -2;
int joyStick_x = A6;
int joyStick_y = A5;
int serialData = 8;
int shiftClock = 9;
int latchClock = 10;
const int row = 8 ;
const int col = 8;
class Node;
class Snake;
class Apple;
class Board;
void play_game(Snake *snake, Apple *apple, Board *board);
void end_game(Snake *snake, Apple *apple, Board *board);

void setup() {
  pinMode(serialData, OUTPUT);
  pinMode(shiftClock, OUTPUT);
  pinMode(latchClock, OUTPUT);
  pinMode(joyStick_x, INPUT);
  pinMode(joyStick_y, INPUT);
  Serial.begin(9600);
}


void write_LED(int matrix[row][col]){
  digitalWrite(shiftClock, LOW);

  for (int i=row-1; i>=0; i--){
    for (int j=col-1; j>=0; j--){
      digitalWrite(latchClock, LOW);
      digitalWrite(serialData, matrix[i][j]);
      digitalWrite(latchClock, HIGH);
    }
  }
  digitalWrite(shiftClock, HIGH);
}

class Node {
  public:
    int x;
    int y;
    Node *prev;
    Node *next;
    Node(int i, int j){
      this->x = j;
      this->y = i;
    }
};

class Snake {
  public:
    Node *head;
    Node *tail;
    int direct;
    int score;
    
    Snake(){
      this->direct = 0;
      this->score = 0;
      Node *newHead = new Node(0,3);
      newHead->next = NULL;
      newHead->prev = NULL;
      this->head = newHead;
      this->tail = newHead;
    }

    void insert_head(int direc){
      Node *newHead = new Node(this->head->y,this->head->x);
      if (direc == right){
        newHead->x++;
      } else if (direc == left){
        newHead->x--;
      } else if (direc == up){
        newHead->y++;
      } else if (direc == down){
        newHead->y--;
      } else {
        return;
      }
      newHead->prev = NULL;
      newHead->next = this->head;
      this->head->prev = newHead;
      this->head = newHead;
      this->score++;
    }

    void delete_tail(){
      Node *temp;
      temp = this->tail;
      this->tail = this->tail->prev;
      this->tail->next = NULL;
      delete temp;
    }

    bool check_collision(){
      //check for hitting border
      if (this->head->x > 7 || 
          this->head->x < 0 ||
          this->head->y > 7 || 
          this->head->y < 0) {
            return true;
      }
      //check for hitting self
      Node *cur = this->head->next;
      while (cur != NULL){
        if (this->head->x == cur->x && this->head->y == cur->y){
          return true;
        }
        cur = cur->next;
      }
      return false;
    }
};

class Apple {
  public:
    int x;
    int y;
    
    Apple(){
      this->x = rand() % (7 - 1) + 1;
      this->y = rand() % (7 - 3) + 3;
    }
    
    void reset(){
      this->x = rand() % (7 - 1) + 1;
      this->y = rand() % (7 - 1) + 1;
    }

    bool check_eaten(Snake* snake){
      if (snake->head->x == this->x && snake->head->y == this->y){
        this->reset();
        return true;
      } else {
        return false;
      }
    }
};

class Board {
  public:
    int matrix[row][col];
    
    void reset_matrix(){
      for (int i = 0; i < row; i++){
        for (int j = 0; j < col; j++){
          this->matrix[i][j] = 0;
        }
      }
    }
    
    void update_matrix(Node *snake_head, Apple* apple, bool show_apple){
      this->reset_matrix();
      if (show_apple){
        this->matrix[apple->y][apple->x] = 1;
      }
      Node *cur = snake_head;
      while (cur != NULL){
        this->matrix[cur->y][cur->x] = 1;
        cur = cur->next;
      }
    }
    
    void print_matrix(){ //debugging
      for (int i = 0; i < row; i++){
        for (int j = 0; j < col; j++){
          Serial.print(this->matrix[i][j]);
        }
        Serial.println("");
      }
      Serial.println("---");
    }
};

int get_move(){
  int val_x = analogRead(joyStick_x);
  int val_y = analogRead(joyStick_y);
  // resting values of our joytstick
  int resting_x = 563;
  int resting_y = 518;
  int range = 50;
  int mov = 0;
  if (val_x < resting_x - range){
    mov = up;
  } else if (val_x > resting_x + range){
    mov = down;
  } else if (val_y < resting_y - range){
    mov = left;
  } else if (val_y > resting_y + range){
    mov = right;
  }
  return mov;
}

void play_game(Snake *snake, Apple *apple, Board *board){
  board->update_matrix(snake->head, apple, true);
  write_LED(board->matrix);
  while (snake->direct == 0){
    int mov = get_move();
    snake->direct = mov;
  }
  while (true){
    delay(125);
    int mov = get_move();
    if (mov != 0 && snake->direct + mov != 0){
      snake->direct = mov;
    }
    snake->insert_head(snake->direct);
    if (snake->check_collision()) {
      break;
    }
    if (apple->check_eaten(snake)) {
    } else {
      snake->delete_tail();
    }
    board->update_matrix(snake->head, apple, true);
    write_LED(board->matrix);
  }
}

//show final snake once game ends
void end_game(Snake *snake, Apple *apple, Board *board){
  Serial.print("Score: ");
  Serial.println(snake->score);
  for (int i = 0; i < 3; i++){
    board->update_matrix(snake->head->next, apple, false);
    write_LED(board->matrix);
    delay(250);
    board->reset_matrix();
    write_LED(board->matrix);
    delay(250);
  }
}

void loop() {
  Snake *snake = new Snake;
  Board *board = new Board;
  Apple *apple = new Apple;
  play_game(snake, apple, board);
  end_game(snake, apple, board);
  //cleanup
  delete snake;
  delete board;
}
