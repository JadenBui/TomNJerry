#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

#define MAX_HEALTH_TOM 5
#define MAX_HEALTH_JERRY 5
#define JERRY_IMG 'J'
#define CHEESE_IMG 'C'
#define TOM_IMG 'T'
#define TRAP_IMG '#'
#define DOOR "X"
#define PI 3.14159265358979323846264338327950288
bool game_over = false;
//define screen values
int W, H;
//define game's values
int cheese, mousetrap, weapon, t;
//define boolean values
bool change, swap, pause, automatic, game_paused, new_game, next_cheese, next_trap, next_firework;
//define trap's vaules
double trap_x[5], trap_y[5];
//define cheese values
double cheese_x[5], cheese_y[5], door_x, door_y;
//define Jerry and Tom stats
int lives_tom, lives_jerry, score_tom, score_jerry, num_firework, cheese_eaten;
//define Jerry values
double jerry_x, jerry_y;
//define Tom values
double tom_x, tom_y, tom_dx, tom_dy, speed;
//define Time values
double current_time;
int time_minute, time_second, pause_time, stop_time, pause_second, count_c, count_t, t_number, count_w, file_number, level_number, count;
//define wall values
double wall_x1[100], wall_y1[100], wall_x2[100], wall_y2[100];
//denfine read values
double jerryX, jerryY, tomX, tomY;
//defind firework values
double firework_x[100], firework_y[100], firework_dx[100], firework_dy[100];
int new_fx[100], new_fy[100];

//READ FILES IN
void read_file_in(FILE *stream)
{
    count_w = 0;
    while (feof(stream) == 0)
    {
        char command;
        double arg1, arg2, arg3, arg4;
        int item_scanned = fscanf(stream, "%c %lf %lf %lf %lf", &command, &arg1, &arg2, &arg3, &arg4);

        if (item_scanned == 3)
        {
            //READ JERRY's LOCATION
            if (command == 'J')
            {
                jerryX = arg1 * (screen_width() - 3) + 1;
                jerryY = arg2 * (screen_height() - 7) + 5;
            }
            //READ TOM's LOCATION
            else if (command == 'T')
            {
                tomX = arg1 * (screen_width() - 3) + 1;
                tomY = arg2 * (screen_height() - 7) + 5;
            }
        }
        if (item_scanned == 5)
        {
            //READ WALL's LOCATION
            if (command == 'W')
            {
                wall_x1[count_w] = arg1 * (screen_width() - 1);
                wall_y1[count_w] = arg2 * (screen_height() - 5) + 5;
                wall_x2[count_w] = arg3 * (screen_width() - 1);
                wall_y2[count_w] = arg4 * (screen_height() - 5) + 5;
                count_w++;
            }
        }
    }
}

//DEFINE COLLIDE DETECTION
bool collide(double x1, double y1, double x2, double y2)
{
    if (round(x1) == round(x2) && round(y1) == round(y2))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//DEFINE COLLIDE WITH RANDOM WALL
bool collide_wall(double x1, double y1)
{
    if (scrape_char(round(x1), round(y1)) == '*')
    {
        return true;
    }
    return false;
}

//DRAW THE BORDER
void draw_border()
{
    draw_line(1, 0, W - 2, 0, '*');
    draw_line(1, H - 1, W - 2, H - 1, '*');
    draw_line(1, 4, W - 2, 4, '*');
    draw_line(0, 0, 0, H - 1, '*');
    draw_line(W - 1, 0, W - 1, H - 1, '*');
}

//DRAW GAME OVER SCREEN
void setup();
void reset_wall();
void game_end()
{
    clear_screen();

    const char *message[] = {
        "GAME OVER!Press 'Q' to EXIT or 'R' to RESTART"};
    int x = (screen_width()) / 2 - 21;
    int y = (screen_height()) / 2 - 3;
    draw_formatted(x, y, *message);
    draw_line(1, 0, W - 2, 0, '*');
    draw_line(1, H - 1, W - 2, H - 1, '*');
    draw_line(0, 0, 0, H - 1, '*');
    draw_line(W - 1, 0, W - 1, H - 1, '*');
    show_screen();

    while (get_char() > 0)
    {
    }
    int choice = wait_char();
    if (choice == 'q')
    {
        game_over = true;
    }

    if (choice == 'r')
    {
        setup();
        reset_wall();
    }
}

//DISPLAY THE STATUS
void draw_status()
{
    //IF press "P" the first time
    if (pause)
    {
        if (game_paused)
        {
            //REMEBER THE EPOC IN SECOND
            stop_time = time_second;
        }
        //UPDATE THE CURRENT TIME
        current_time = get_current_time();
    }

    if (!pause) //IF GAME NOT PAUSED
    {
        //GET THE GET_CURREN_TIME TO MINUS THE GAME START TIME (current_time is DECLARED IN THE SETUP)//
        //AND IF PAUSED RESET THE TIME_SECOND TO 0 AND PLUS THE SECOND WHEN STOPPED.
        time_second = get_current_time() - current_time + stop_time;
    }

    draw_formatted(1, 1, "Student Number: n10560564             Score: %1d            Lives: %1d            Player: %s", (swap ? score_tom : score_jerry), (swap ? lives_tom : lives_jerry), (swap ? "Tom" : "Jerry"));

    draw_formatted(1, 2, "Cheese: %1d                           MouseTrap: %1d        Fireworks:%s%1d        Step: %1lf", count_c, count_t, (num_firework == 0 ? "Out-of-bullet:" : ""), num_firework, speed);

    draw_formatted(1, 3, "Level: %d                             Time: %02d:%02d ", level_number + 1, time_second / 60, time_second % 60);

    //DISPLAY SCORE WHEN JERRY EAT ENOUGH CHEESE
}
//DRAW WALL
void draw_wall()
{
    for (int i = 0; i <= count_w; i++)
    {
        draw_line(round(wall_x1[i]), round(wall_y1[i]), round(wall_x2[i]), round(wall_y2[i]), '*');
    }
}

//INITIALIZE VALUE FOR JERRY
void jerry_default()
{
    jerry_x = jerryX;
    jerry_y = jerryY;
}

void setup_jerry()
{
    jerry_default();
    score_jerry = 0;
    lives_jerry = MAX_HEALTH_JERRY;
}

void draw_jerry()
{
    //IF SWITCH PLAYER THEN DISPLAY DIFFERENT IMAGE
    if (swap)
    {
        draw_char(round(jerry_x), round(jerry_y), TOM_IMG);
    }
    else
    {
        draw_char(round(jerry_x), round(jerry_y), JERRY_IMG);
    }
}

void update_jerry(int key)
{
    if (key == 'a' && jerry_x > 1 && !(collide_wall(jerry_x - 1, jerry_y)))
    {
        jerry_x--;
    }
    if (key == 'd' && jerry_x < W - 2 && !(collide_wall(jerry_x + 1, jerry_y)))
    {
        jerry_x++;
    }
    if (key == 'w' && jerry_y > 5 && !(collide_wall(jerry_x, jerry_y - 1)))
    {
        jerry_y--;
    }
    if (key == 's' && jerry_y < H - 2 && !(collide_wall(jerry_x, jerry_y + 1)))
    {
        jerry_y++;
    }

    if (lives_jerry == 0)
    {
        game_end();
    }
}

void draw_tom()
{
    if (swap)
    {
        draw_char(round(tom_x), round(tom_y), JERRY_IMG);
    }
    else
    {
        draw_char(round(tom_x), round(tom_y), TOM_IMG);
    }
}

//INITIALIZE VALUE FOR TOM

void tom_default()
{
    tom_x = tomX;
    tom_y = tomY;
}

void setup_tom()
{
    tom_default();
    score_tom = 0;
    lives_tom = MAX_HEALTH_TOM;
    double step = speed;
    double tom_dir = rand() * PI * 2 / RAND_MAX;
    tom_dx = step * cos(tom_dir);
    tom_dy = step * sin(tom_dir);
}

//CALCULATE DISTANCE WITH DOUBLE CORDINATOR
void distance_tom_w_double(double target_x, double target_y)
{
    double step = speed;
    double t1 = target_x - tom_x;
    double t2 = target_y - tom_y;
    double d = sqrt(t1 * t1 + t2 * t2);
    tom_dx = step * (t1 * 0.35) / d;
    tom_dy = step * (t2 * 0.35) / d;
}

//CALCULATE DISTANCE WITH DOUBLE ARRAY OF CORDINATOR
void distance_tom_w_array(double target_x[], double target_y[], int index)
{
    double step = speed;
    double t1 = target_x[index] - tom_x;
    double t2 = target_y[index] - tom_y;
    double d = sqrt(t1 * t1 + t2 * t2);
    tom_dx = step * (t1 * 0.35) / d;
    tom_dy = step * (t2 * 0.35) / d;
}

bool move_tom()
{
    if (swap)
    {
        //IF JERRY CLOSE TO TOM : EVADE
        if (abs(tom_x - jerry_x) <= 3 && abs(tom_y - jerry_y) <= 3)
        {
            double step = 1;
            double t1 = tom_x - jerry_x;
            double t2 = tom_x - jerry_y;

            double d = sqrt(t1 * t1 + t2 * t2);
            tom_dx = step * (t1 * 0.35) / d;
            tom_dy = step * (t2 * 0.35) / d;
        }
    }
    else
    {
        //IF TOM CLOSE JERRY : ENCREASE SPEED AND UPDATE CHASING PATH
        if (abs(round(tom_x - jerry_x)) <= 3 && abs(round(tom_y - jerry_y)) <= 3)
        {
            speed = 1;
            distance_tom_w_double(jerry_x, jerry_y);
        }
    }

    int new_x = round(tom_x + tom_dx);
    int new_y = round(tom_y + tom_dy);
    bool bounced = false;
    //BOUNCE TO BORDER AND ARBITRARY WALLS
    if (new_x == 0 || new_x == W - 1 || new_x < 0 || new_x > W - 1 || collide_wall(new_x, tom_y))
    {
        bounced = true;
        tom_dx = -tom_dx;
        change = !(change);
        return true;
    }

    if (new_y == 4 || new_y == H - 1 || new_y < 4 || new_y > H - 1 || collide_wall(tom_x, new_y))
    {
        bounced = true;
        tom_dy = -tom_dy;
        change = !(change);
        return true;
    }

    if (!bounced)
    {
        tom_x += tom_dx;
        tom_y += tom_dy;
        return false;
    }
    return false;
}
//IF COLLIDE WITH THE WALL
void change_speed(bool collide)
{
    if (collide)
    {
        //CHANGE SPEED
        speed = (double)rand() / RAND_MAX * 0.7 + 0.5;

        //AI TOM WILL SEEK AFTER FIRST BOUNCE, UN_SEEK FOR JERRY TO RUN IN SECOND TIME//
        //SEEK AGAIN IN THE THIRD TIME(LIKE A SWITCH)
        if (change && !(swap))
        {
            distance_tom_w_double(jerry_x, jerry_y);
        }
    }
}

void update_tom(int key)
{
    bool isCollide = false;
    //IF NOT PAUSE : MOVE
    if (!pause)
    {
        if (key < 0)
        {
            isCollide = move_tom();
            change_speed(isCollide);
        }
    }
    //COLLIDE WITH JERRY
    if (!swap)
    {
        if (collide(tom_x, tom_y, jerry_x, jerry_y))
        {
            jerry_default();
            tom_default();
            lives_jerry--;
        }
    }
    else
    {
        if (collide(tom_x, tom_y, jerry_x, jerry_y))
        {
            jerry_default();
            tom_default();
            score_tom += 5;
        }
    }

    if (lives_tom == 0)
    {
        game_end();
    }
}

//CHECK IF THE CHEESE COLLIDE WITH OTHER ELEMENTS
bool cheese_collision(int i)
{
    if (collide_wall(cheese_x[i], cheese_y[i]) || collide(cheese_x[i], cheese_y[i], tom_x, tom_y) || collide(cheese_x[i], cheese_y[i], jerry_x, jerry_y) || collide(cheese_x[i], cheese_y[i], cheese_x[i - 1], cheese_y[i - 1]))
    {
        return true;
    }
    return false;
}

//INITIALIZE VALUE FOR CHEESE
void setup_cheese()
{
    //INITIAL RANDOM CHEESE LOCATION
    cheese = 0;
    cheese_eaten = 0;
    count_c = 0;
    for (int i = 0; i < 5; i++)
    {
        cheese_x[i] = 1 + rand() % (W - 2);
        cheese_y[i] = 5 + rand() % (H - 6);
        //AVOID THE COLLISION WITH OTHER ELEMENTS
        while (cheese_collision(i))
        {
            cheese_x[i] = 1 + rand() % (W - 2);
            cheese_y[i] = 5 + rand() % (H - 6);
        }
    }
}

//SET UP CHEESE THAT DETECTED COLLISION ON
void setup_cheeseOnCollide(int position)
{
    cheese_x[position] = 1 + rand() % (W - 2);
    cheese_y[position] = 5 + rand() % (H - 6);
    while (cheese_collision(position))
    {
        cheese_x[position] = 1 + rand() % (W - 2);
        cheese_y[position] = 5 + rand() % (H - 6);
    }
}

void draw_cheese()
{
    //ONLY AUTOMATICALLY SPAWN IN THE 1 LEVEL AS REQUIREMENTS
    if (level_number == 0)
    {
        //SPAW AFTER 2 SECONDS EACH
        if (time_second >= 2 && time_second % 2 == 0 && next_cheese && count_c < 5)
        {
            count_c++;
            next_cheese = false;
        }

        else if (time_second % 2 != 0)
        {
            next_cheese = true;
        }
    }
    //DRAW THE CHEESE
    for (int i = 0; i < count_c; i++)
    {
        draw_char(cheese_x[i], cheese_y[i], CHEESE_IMG);
    }
}
//COLIDE DETECTION FOR THE CHEESE WITH CHARACTERS
void update_cheese(int key)
{
    if (swap)
    {
        for (int i = 0; i < 5; i++)
        {
            if (collide(tom_x, tom_y, cheese_x[i], cheese_y[i]))
            {
                score_jerry++;
                setup_cheeseOnCollide(i);
            }
        }
    }

    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (collide(jerry_x, jerry_y, cheese_x[i], cheese_y[i]))
            {
                score_jerry++;
                cheese_eaten++;
                setup_cheeseOnCollide(i);
            }
        }
    }
}
//RESET POSITION OF WALL AFTER EACH LEVEL TO AVOID OVERLAP
void reset_wall()
{
    memset(wall_x1, 0, sizeof(wall_x1));
    memset(wall_x2, 0, sizeof(wall_x2));
    memset(wall_y1, 0, sizeof(wall_y1));
    memset(wall_y2, 0, sizeof(wall_y2));
}

void setup();
void setup_firework(int num_firework);
void setup_trap(int trap_number);
//UPDATE THE GAME BASE ON THE INPUT KEY
void update_game(int key)
{
    if (key == 'z' && level_number >= 1)
    {
        swap = !(swap);
    }

    if (key == 'p')
    {
        pause = !(pause);
        game_paused = !(game_paused);
    }

    if (key == 'r')
    {
        setup();
        reset_wall();
        swap = false;
    }

    if (key == 'l')
    {
        level_number++;
        new_game = !(new_game);
        if (level_number < file_number - 2)
        {
            reset_wall();
            count_t = 0;
            count_c = 0;
            cheese_eaten = 0;
            cheese = 0;
            t_number = 0;
            for (int i = 0; i < 5; i++)
            {
                cheese_x[i] = -1;
                cheese_y[i] = -1;
            }
        }
        swap = false;
    }

    if (key == 'c' && level_number != 0)
    {
        if (swap)
        {
            cheese_x[cheese] = round(jerry_x);
            cheese_y[cheese] = round(jerry_y);
            //CHECK COLLISION WITH OTHER CHEESE AND GAME ELEMENTS
            while (scrape_char(cheese_x[cheese], cheese_y[cheese]) == 'C' || collide_wall(cheese_x[cheese], cheese_y[cheese]) || scrape_char(cheese_x[cheese], cheese_y[cheese]) == '#')
            {
                cheese_y[cheese] += 1;
            }

            //JERRY AUTOMATICALLY SEEK FOR PLAYER's CHEESE PLACEMENT
            speed = 1.2;
            distance_tom_w_array(cheese_x, cheese_y, cheese);
            if (count_c < 5)
            {
                count_c++;
            }

            if (cheese < 5)
            {
                cheese++;
            }

            if (cheese == 5)
            {
                cheese = 0;
            }
        }
    }

    if (key == 'm' && level_number != 0)
    {
        if (swap)
        {
            automatic = (!automatic);
            trap_x[t_number] = round(jerry_x);
            trap_y[t_number] = round(jerry_y);
            //CHECK COLLISION WITH OTHER TRAPS AND GAME ELEMENTS
            while (scrape_char(trap_x[t_number], trap_y[t_number]) == '#' || collide_wall(trap_x[t_number], trap_y[t_number]) || scrape_char(trap_x[t_number], trap_y[t_number]) == 'C')
            {
                trap_y[t_number] += 1;
            }
            t_number++;
            if(count_t < 5){
                count_t += 1;
            }

            if(t_number== 5){
                t_number -=5;
            }
        }
    }

    if (key == 'f' && level_number >= 1)
    {
        if (!swap && num_firework > 0)
        {
            num_firework--;
            setup_firework(num_firework);
        }
    }

    if (key == 'q')
    {
        game_end();
    }
}

void game_end();

void setup_door()
{
    door_x = 1 + rand() % (W - 2);
    door_y = 5 + rand() % (H - 6);
    //CHECK COLLISION BETWEEN DOOR AND OTHER GAME ELEMENTS
    while (collide_wall(door_x, door_y) || collide(door_x, door_y, tom_x, tom_y) || collide(door_x, door_y, jerry_x, jerry_y))
    {
        door_x = 1 + rand() % (W - 2);
        door_y = 5 + rand() % (H - 6);
    }
}

void update_door()
{
    //IF PLAYER CONTROLLED CHARACTER ENTER DOOR : GO TO NEXT LEVEL
    if (collide(door_x, door_y, jerry_x, jerry_y))
    {
        level_number++;
        if (level_number < file_number - 2)
        {
            new_game = !(new_game);
            jerry_default();
            tom_default();
            cheese_eaten = 0;
            count_t = 0;
            count_c = 0;
            reset_wall();
            swap = false;
            for (int i = 0; i < 5; i++)
            {
                cheese_x[i] = -1;
                cheese_y[i] = -1;
            }
        }
    }
}

void draw_door()
{
    if (cheese_eaten >= 5)
    {
        draw_formatted(door_x, door_y, DOOR);
        draw_formatted(door_x - 1, door_y, "[");
        draw_formatted(door_x + 1, door_y, "]");
    }
}

// void setup_trap_initial(){
//     for (int i = 0; i < 5; i++)
//     {
//         trap_x[i] = -1;
//         trap_y[i] = -1;
//     }
// }

void setup_trap(int trap_number)
{
    //SET UP TRAP ON AI TOM PATH
    if (!swap)
    {
        trap_x[trap_number] = round(tom_x);
        trap_y[trap_number] = round(tom_y);
    }
    //SET UP TRAP ON PLAYER TOM PATH
    else
    {
        trap_x[trap_number] = round(jerry_x);
        trap_y[trap_number] = round(jerry_y);
    }
}

void draw_trap()
{
    if (!swap)
    {
        //DRAW TRAP AFTER 3 SECOND EACH.
        if (time_second >= 3 && time_second % 3 == 0 && next_trap && count_t < 5)
        {
            setup_trap(count_t);
            count_t++;
            next_trap = false;
        }

        else if (time_second % 3 != 0)
        {
            next_trap = true;
        }
    }

    for (int i = 0; i < count_t; i++)
    {
        draw_char(trap_x[i], trap_y[i], TRAP_IMG);
    }
}

void update_trap()
{
    if (!swap)
    {
        for (int i = 0; i < 5; i++)
        {
            //SET UP THE TRAP THAT COLLIDE WITH THE CHARACTER
            if (collide(jerry_x, jerry_y, trap_x[i], trap_y[i]))
            {
                jerry_default();
                setup_trap(i);
                lives_jerry--;
            }
        }
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (collide(tom_x, tom_y, trap_x[i], trap_y[i]))
            {
                tom_default();
                setup_trap(i);
                lives_jerry--;
                score_tom++;
            }
        }
    }
}

void setup_time()
{
    stop_time = 0;
    time_second = 0;
    time_minute = 0;
    pause_time = 0;
    pause_second = 0;
    game_paused = pause = swap = false;
    automatic = true;
}

void setup_firework_values()
{
    num_firework = 100;
}

//SPAWN FIREWORK BASED ON BOTH PLAYER AND AI JERRY LOCATION
void setup_firework(int firework_number)
{
    if (!swap)
    {
        firework_x[firework_number] = round(jerry_x);
        firework_y[firework_number] = round(jerry_y);
        double step = 0.8;
        double t1 = tom_x - firework_x[firework_number];
        double t2 = tom_y - firework_y[firework_number];
        double d = sqrt(t1 * t1 + t2 * t2);

        firework_dx[firework_number] = step * (t1 * 0.5) / d;
        firework_dy[firework_number] = step * (t2 * 0.5) / d;
    }
    else
    {
        firework_x[firework_number] = round(tom_x);
        firework_y[firework_number] = round(tom_y);
        double step = 0.8;
        double t1 = jerry_x - firework_x[firework_number];
        double t2 = jerry_y - firework_y[firework_number];
        double d = sqrt(t1 * t1 + t2 * t2);

        firework_dx[firework_number] = step * (t1 * 0.5) / d;
        firework_dy[firework_number] = step * (t2 * 0.5) / d;
    }
}

void draw_firework()
{
    for (int i = 99; i >= 0; i--)
    {
        draw_char(round(firework_x[i]), round(firework_y[i]), '>');
    }
}

void update_firework()
{
    if (swap)
    {
        //AUTOMATICALLY FIRE AFTER 5 SECOND EACH
        if (time_second % 5 == 0 && next_firework && num_firework > 0)
        {
            num_firework--;
            setup_firework(num_firework);
            next_firework = false;
        }

        if (time_second % 5 != 0)
        {
            next_firework = true;
        }
    }
    bool collide_firework = false;
    //UPDATE THE COLLISION OF FIREWORK AND CHARACTER, WALL, BORDER.
    for (int i = 99; i >= 0; i--)
    {
        //IF FIREWORK NEAR TOM : UPDATE SPEED AND DIRECTION FOR MORE AGRESSIVE CHASING
        if (round(abs(firework_x[i] - ((swap) ? jerry_x : tom_x))) <= 5 || round(abs(firework_x[i] - ((swap) ? jerry_y : tom_y))) <= 5)
        {
            double step = 1.2;
            double t1 = ((swap) ? jerry_x : tom_x) - firework_x[i];
            double t2 = ((swap) ? jerry_y : tom_y) - firework_y[i];

            double d = sqrt(t1 * t1 + t2 * t2);
            firework_dx[i] = step * (t1 * 0.35) / d;
            firework_dy[i] = step * (t2 * 0.35) / d;
        }

        new_fx[i] = round(firework_x[i] + firework_dx[i]);
        new_fy[i] = round(firework_y[i] + firework_dy[i]);

        //IF FIREWORK HITS WALL
        if (collide_wall(new_fx[i], new_fy[i]) || new_fx[i] == 0 || new_fx[i] == W - 1 || new_fy[i] == 4 || new_fy[i] == H - 1)
        {
            firework_x[i] = -1;
            firework_y[i] = -1;
            collide_firework = true;
        }

        //IF FIREWORK HITS CHARACTER
        if (swap)
        {
            if (collide(firework_x[i], firework_y[i], jerry_x, jerry_y))
            {
                lives_tom--;
                score_jerry++;
                firework_x[i] = -1;
                firework_y[i] = -1;
                collide_firework = true;
                jerry_default();
            }
        }
        else
        {
            if (collide(firework_x[i], firework_y[i], tom_x, tom_y))
            {
                lives_tom--;
                score_jerry++;
                firework_x[i] = -1;
                firework_y[i] = -1;
                collide_firework = true;
                tom_default();
            }
        }
        //MOVE ON IF NO COLLISION DETECTED
        if (!(collide_firework))
        {
            firework_x[i] += firework_dx[i];
            firework_y[i] += firework_dy[i];
        }
    }
}
//SET UP GAME
void setup()
{
    W = screen_width();
    H = screen_height();
    srand(get_current_time());
    current_time = get_current_time();
    setup_firework_values();
    setup_time();
    level_number = 0;
    count_t = 0;
    new_game = true;
    next_cheese = true;
    next_trap = true;
    next_firework = true;
    change = true;
    speed = 0.2;
    mousetrap = 0;
    weapon = 0;
    t_number = 0;
    setup_jerry();
    setup_cheese();
    setup_tom();
    setup_door();
}

void draw_all()
{
    clear_screen();
    draw_border();
    draw_wall();
    draw_status();
    draw_jerry();
    draw_tom();
    draw_trap();
    draw_door();
    draw_firework();
    draw_cheese();
    show_screen();
}
// UPDATE GAME ELEMENTS
void loop()
{
    int key = get_char();
    update_jerry(key);
    update_cheese(key);
    update_tom(key);
    update_game(key);
    update_door();
    update_firework();
    update_trap();
}

int main(int argc, char *argv[])
{
    setup_screen();
    //CREATE ARRAY OF FILES NAME
    char *files[100];
    file_number = 0;
    for (int i = 0; i < argc; i++)
    {
        files[i] = argv[i + 1];
        file_number++;
    }
    setup();

    while (!game_over)
    {
        //READ THE LEVEL GIVEN
        FILE *stream = fopen(files[level_number], "r");

        if (stream != NULL)
        {
            read_file_in(stream);
            fclose(stream);
        }
        //IF SWITCH LEVEL, UPDATE THE GAME's VALUES
        draw_all();
        if (new_game)
        {
            if (level_number == 0)
            {
                setup_cheese();
            }
            jerry_default();
            tom_default();
            setup_door();
            new_game = !(new_game);
        }

        loop();
        timer_pause(10);
    }
    return 0;
}