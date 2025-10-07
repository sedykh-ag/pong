#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

#define FRAME_WIDTH 47
#define FRAME_HEIGHT 11
#define PADDLE_LENGTH 3
#define DT 80

#ifdef _WIN32
void msleep(unsigned int ms) { Sleep(ms); }

static DWORD original_console_mode;

void set_canon_terminal_mode() {
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(hStdin, &original_console_mode);
  SetConsoleMode(hStdin,
    original_console_mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
  printf("\x1b[?25l"); // hide cursor
}

void unset_canon_terminal_mode() {
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  SetConsoleMode(hStdin, original_console_mode);
  printf("\x1b[?25h"); // show cursor
}
#else
void msleep(unsigned int ms) { usleep(ms * 1000u); }

static struct termios original_terminal_mode;
static int original_file_flags;

void set_canon_terminal_mode() {
  tcgetattr(STDIN_FILENO, &original_terminal_mode);
  struct termios new_terminal_mode = original_terminal_mode;
  new_terminal_mode.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal_mode);

  original_file_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, original_file_flags | O_NONBLOCK);
}

void unset_canon_terminal_mode() {
  tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_mode);
  fcntl(STDIN_FILENO, F_SETFL, original_file_flags);
}

int _kbhit() { return 1; }
#endif

int key_hit()
{
  return _kbhit();
}

struct Ball
{
  unsigned x, y;
  unsigned v_x, v_y;
};

struct Paddle
{
  unsigned x;
  unsigned y_top, y_bot;
};

void reset_ball(struct Ball *ball)
{
  ball->x = FRAME_WIDTH / 2; ball->y = FRAME_HEIGHT / 2;
  ball->v_x = (rand() % 2) == 0 ? -1 : 1;
  ball->v_y = (rand() % 2) == 0 ? -1 : 1;
}

int main()
{
  srand(time(NULL));

  struct Ball ball;
  reset_ball(&ball);

  struct Paddle rPaddle;
  rPaddle.x = FRAME_WIDTH - 4;
  rPaddle.y_top = (FRAME_HEIGHT - 1) / 2;
  rPaddle.y_bot = rPaddle.y_top + (PADDLE_LENGTH - 1);

  struct Paddle lPaddle;
  lPaddle.x = 3;
  lPaddle.y_top = (FRAME_HEIGHT - 1) / 2;
  lPaddle.y_bot = rPaddle.y_top + (PADDLE_LENGTH - 1);

  unsigned lScore = 0, rScore = 0;

  set_canon_terminal_mode();

  while (1)
  {
    // controls
    if (key_hit())
    {
      int c = getchar();
      // quit
      if (c == 'q')
        break;
      // right paddle
      else if (c == 'l' && rPaddle.y_bot < (FRAME_HEIGHT - 2))
      {
        rPaddle.y_top += 1;
        rPaddle.y_bot += 1;
      }
      else if (c == 'o' && rPaddle.y_top > 1)
      {
        rPaddle.y_top -= 1;
        rPaddle.y_bot -= 1;
      }
      // left paddle
      else if (c == 's' && lPaddle.y_bot < (FRAME_HEIGHT - 2))
      {
        lPaddle.y_top += 1;
        lPaddle.y_bot += 1;
      }
      else if (c == 'w' && lPaddle.y_top > 1)
      {
        lPaddle.y_top -= 1;
        lPaddle.y_bot -= 1;
      }
    }

    // kinematics
    if (ball.y <= 1 || ball.y >= (FRAME_HEIGHT - 2))
      ball.v_y = -ball.v_y;
    if (ball.x <= 0)
    {
      rScore++;
      reset_ball(&ball);
    }
    else if (ball.x >= (FRAME_WIDTH - 1))
    {
      lScore++;
      reset_ball(&ball);
    }

    if (ball.x == (rPaddle.x - 1) && ball.y >= rPaddle.y_top && ball.y <= rPaddle.y_bot)
      ball.v_x = -ball.v_x;

    if (ball.x == (lPaddle.x + 1) && ball.y >= lPaddle.y_top && ball.y <= lPaddle.y_bot)
      ball.v_x = -ball.v_x;

    ball.x += ball.v_x;
    ball.y += ball.v_y;

    // rendering
    printf("\033[H\033[J"); // move cursor to top-left position

    printf("q to quit\n"
           "w/d to move left paddle\n"
           "o/l to move right paddle\n\n");

    printf("SCORE %u %u\n", lScore, rScore);
    for (unsigned y = 0; y < FRAME_HEIGHT; y++)
    {
      for (unsigned x = 0; x < FRAME_WIDTH; x++)
      {
        // empty
        char c = ' ';

        // frame
        if (y == 0 || y == (FRAME_HEIGHT - 1))
          c = '-';
        else if (x == 0 || x == (FRAME_WIDTH - 1))
          c = '|';

        // ball
        if (x == ball.x && y == ball.y)
          c = 'o';

        // right paddle
        if ((x == rPaddle.x) && (y <= rPaddle.y_bot) && (y >= rPaddle.y_top))
          c = '|';

        // left paddle
        if ((x == lPaddle.x) && (y <= lPaddle.y_bot) && (y >= lPaddle.y_top))
          c = '|';

        putchar(c);
      }
      putchar('\n');
    }
    msleep(DT);
  }

  unset_canon_terminal_mode();

  return 0;
}
