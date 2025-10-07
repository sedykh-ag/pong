#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define FRAME_WIDTH 45
#define FRAME_HEIGHT 10
#define PADDLE_LENGTH 3
#define DT 100

void msleep(unsigned int ms)
{
#ifdef _WIN32
  Sleep(ms);
#else
  usleep(ms * 1000u);
#endif
}

void set_canon_terminal_mode() {
  struct termios terminal;
  tcgetattr(STDIN_FILENO, &terminal);
  terminal.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal);

  int file_status_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, file_status_flags | O_NONBLOCK);
}

void unset_canon_terminal_mode() {
  struct termios new_termios;
  tcgetattr(STDIN_FILENO, &new_termios);
  new_termios.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

  int file_status_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, file_status_flags & (~O_NONBLOCK));
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

int main()
{
  struct Ball ball;
  ball.x = FRAME_WIDTH / 2; ball.y = FRAME_HEIGHT / 2;
  ball.v_x = 1; ball.v_y = 1;

  struct Paddle rPaddle;
  rPaddle.x = FRAME_WIDTH - 4;
  rPaddle.y_top = (FRAME_HEIGHT - 1) / 2;
  rPaddle.y_bot = rPaddle.y_top + (PADDLE_LENGTH - 1);

  struct Paddle lPaddle;
  lPaddle.x = 3;
  lPaddle.y_top = (FRAME_HEIGHT - 1) / 2;
  lPaddle.y_bot = rPaddle.y_top + (PADDLE_LENGTH - 1);

  set_canon_terminal_mode();

  while (1)
  {
    // controls
    int c = getchar();
    if (c != EOF)
    {
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
    if (ball.x <= 1 || ball.x >= (FRAME_WIDTH - 2))
      ball.v_x = -ball.v_x;
    if (ball.y <= 1 || ball.y >= (FRAME_HEIGHT - 2))
      ball.v_y = -ball.v_y;

    if (ball.x == (rPaddle.x - 1) && ball.y >= rPaddle.y_top && ball.y <= rPaddle.y_bot)
      ball.v_x = -ball.v_x;

    if (ball.x == (lPaddle.x + 1) && ball.y >= lPaddle.y_top && ball.y <= lPaddle.y_bot)
      ball.v_x = -ball.v_x;

    ball.x += ball.v_x;
    ball.y += ball.v_y;

    // rendering
    printf("\033[H\033[J");
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
          c = '*';

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
