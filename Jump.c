#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <time.h>

#define WINDOW_W 1000
#define WINDOW_H 600
#define NUM_PLATFORM 5
#define MAXENERGY 800
#define ORIXPLAYER 383
#define ORIYPLAYER 383
#define ORIXPLATFORM 285
#define ORIYPLATFORM 400
#define MAXREDUCTION 100 // platform must be bigger than the player

#undef main

int record = 0;
int score = 0;
int score_temp = 0; // delay the score change until after landing
int status = 1;
int status_temp = 0; // delay the status/parachute number change until after landing
int cur_type = 0;
int cur_size = 0;
int nxt_type = 0;
int nxt_size = 0;
int distance = 0;
int direction = 0;
int landing;
int sensitivity = 3;
double delta_dir[2]; // distance from the center of platform
double delta_dir_temp[2];

SDL_Window *window;
SDL_Renderer *renderer;
Mix_Chunk *jump_au;
Mix_Chunk *fall_au;
Mix_Chunk *perfect_au;
Mix_Chunk *fall_au;
Mix_Chunk *click_au;
Mix_Music *bgm;
SDL_Surface *player_surface;
SDL_Texture *player_texture;
SDL_Surface *platform_surface[NUM_PLATFORM];
SDL_Texture *platform_texture[NUM_PLATFORM];
SDL_Surface *scoreboard_surface;
SDL_Texture *scoreboard_texture;
SDL_Surface *button_auto_surface;
SDL_Texture *button_auto_texture;
SDL_Surface *parachute_surface;
SDL_Texture *parachute_texture;
TTF_Font *comic;

void init();
void shutdown();
void menu();                                                                    // manage menu events
void game();                                                                    // main game events
void gen_platform();                                                            // randomly create the next platform
int get_energy();                                                               // get the amount of energy built up by holding down the key
void jump(int energy, int mode);                                                // display player during jump
void land();                                                                    // display different landing accordingly
void update_status();                                                           // update status after landing
void update_location();                                                         // update location after jump
void hop_and_update(int energy);                                                // hop and update location
int check_cur(int energy);                                                      // check whether the energy built up is enough to leave the current platform
int check_nxt();                                                                // check whether the player lands on the platform
void if_record();                                                               // check if the game breaks a record
void move();                                                                    // remove to center page
void next();                                                                    // set the page for next jump
void moftplayer(int level);                                                     // alter player shape while building up energy
void dp_background();                                                           // show background elements
void dp_energy(int amount);                                                     // display the amount of energy built up while building up energy
void dp_player(double loc, int dir, int size, int height);                      // display player when on platform
void dp_player_jump(double i, int dir, int mode, int energy);                   // display the player during jump
void dp_platform(int loc, int dir, int type, int size, int x_move, int y_move); // display platform
void dp_parachute(double loc, int dir, int size, int height);                   // display parachute when saving the player
void dp_menu(int a, int b);                                                     // display menu
void dp_settings();                                                             // display settings page
void dp_readme(int pg);                                                         // display introduction page

int main()
{
    init();
    atexit(shutdown);
    menu();
    return 0;
}

void menu()
/*manage menu events*/
{
    dp_menu(0, 0);
    SDL_RenderPresent(renderer);
    SDL_Event start;
    int fla = 1;
    while (fla)
    {
        while (SDL_PollEvent(&start))
        {
            if ((start.type == SDL_MOUSEBUTTONDOWN && (start.button.x - 500) * (start.button.x - 500) + (start.button.y - 300) * (start.button.y - 300) < 1500) || (start.type == SDL_KEYDOWN && start.key.keysym.sym == SDLK_RETURN))
            {
                fla = 0;
                status = 1;
                score = 0;
                cur_type = 0;
                cur_size = 0;
                delta_dir[0] = 0;
                delta_dir[1] = 0;
                Mix_PlayMusic(bgm, 4);
                game();
            }
            if ((start.type == SDL_KEYDOWN && start.key.keysym.sym == SDLK_ESCAPE) || (start.type == SDL_QUIT) || (start.type == SDL_MOUSEBUTTONDOWN && start.button.x > 456 && start.button.x < 545 && start.button.y > 402 && start.button.y < 453))
                exit(0);

            // Readme page
            if (start.type == SDL_MOUSEBUTTONDOWN && (start.button.x - 500) * (start.button.x - 500) + (start.button.y - 300) * (start.button.y - 300) > 40000 && (start.button.x - 380) * (start.button.x - 380) + (start.button.y - 300) * (start.button.y - 300) < 22500)
            {
                Mix_PlayChannel(-1, click_au, 0);
                dp_menu(0, 1);
                dp_readme(1);
                SDL_RenderPresent(renderer);
                SDL_Event read;
                int f = 1;
                while (f < 4)
                {
                    while (SDL_PollEvent(&read))
                    {
                        switch (read.type)
                        {
                        case SDL_MOUSEBUTTONDOWN:
                            if (((read.button.x - 500) * (read.button.x - 500) + (read.button.y - 300) * (read.button.y - 300) < 40000) || (read.button.x > 205 && read.button.x < 265 && read.button.y > 435 && read.button.y < 465 && f == 3))
                            {
                                f = 4;
                                Mix_PlayChannel(-1, click_au, 0);
                                dp_menu(0, 0);
                                SDL_RenderPresent(renderer);
                                break;
                            }
                            if (read.button.x > 205 && read.button.x < 265 && read.button.y > 435 && read.button.y < 465 && f <= 2)
                            {
                                f++;
                                Mix_PlayChannel(-1, click_au, 0);
                                dp_menu(0, 1);
                                dp_readme(f);
                                SDL_RenderPresent(renderer);
                                break;
                            }
                            break;
                        case SDL_QUIT:
                            exit(0);
                            break;
                        }
                    }
                }
            }

            // settings page
            if (start.type == SDL_MOUSEBUTTONDOWN && (start.button.x - 500) * (start.button.x - 500) + (start.button.y - 300) * (start.button.y - 300) > 40000 && (start.button.x - 620) * (start.button.x - 620) + (start.button.y - 300) * (start.button.y - 300) < 22500)
            {
                Mix_PlayChannel(-1, click_au, 0);
                dp_menu(1, 0);
                dp_settings();
                SDL_RenderPresent(renderer);
                SDL_Event set;
                int f = 1;
                while (f)
                {
                    while (SDL_PollEvent(&set))
                    {
                        switch (set.type)
                        {
                        case SDL_MOUSEBUTTONDOWN:
                            if ((set.button.x > 745 && set.button.x < 805 && set.button.y > 435 && set.button.y < 465) || ((set.button.x - 500) * (set.button.x - 500) + (set.button.y - 300) * (set.button.y - 300) < 40000))
                            {
                                f = 0;
                                Mix_PlayChannel(-1, click_au, 0);
                                dp_menu(0, 0);
                                SDL_RenderPresent(renderer);
                                break;
                            }
                            if (set.button.y > 190 && set.button.y < 340 && set.button.x > 720 && set.button.x < 870)
                            {
                                sensitivity = (set.button.y - 190) / 30 + 1;
                                Mix_PlayChannel(-1, click_au, 0);
                                dp_menu(1, 0);
                                dp_settings();
                                SDL_RenderPresent(renderer);
                            }
                            break;
                        case SDL_QUIT:
                            exit(0);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void game()
/* manage main game event*/
{
    dp_background();
    gen_platform();
    dp_platform(0, 0, 0, 0, 0, 0);
    dp_player(0, 0, 0, 0);
    SDL_RenderPresent(renderer);
    while (status >= 1)
    {
        int energy;
        int flg = 1; // whether or not jump leaves the current platform
        while (flg)
        {
            energy = get_energy();
            energy = energy > MAXENERGY ? MAXENERGY : energy;
            flg = check_cur(energy);
            SDL_Delay(5);
        }
        delta_dir_temp[1 - direction] = 0;
        delta_dir_temp[direction] = (energy - (distance - delta_dir[direction]) * 10) / 10.0;
        status_temp += check_nxt();
        jump(energy, 1);
        update_location();
        land();
        update_status();
        if (status < 1)
        {
            Mix_FadeOutMusic(300);
            if_record();
            menu();
        }
        move();
        next();
    }
}

void gen_platform()
/*create a new platform*/
{
    direction = (int)rand() % 2;
    distance = 25 + (int)rand() % 23;
    nxt_type = (int)rand() % NUM_PLATFORM;
    nxt_size = score < MAXREDUCTION ? score : MAXREDUCTION; // if bigger than the limit, reduce in size
    dp_platform(distance, direction, nxt_type, nxt_size, 0, 0);
}

int get_energy()
/*get the amount of energy built up*/
{
    Uint32 starttime, endtime;
    SDL_Event prejump;
    int fl = 1;
    int energy = 0;
    const Uint8 *state;
    while (fl)
    {
        while (SDL_PollEvent(&prejump))
        {
            if (prejump.type == SDL_KEYDOWN && prejump.key.keysym.sym == SDLK_SPACE && fl == 1)
            {
                starttime = SDL_GetTicks();
                fl = 2;
            }
            if (prejump.type == SDL_KEYUP && prejump.key.keysym.sym == SDLK_SPACE && fl == 2)
            {
                endtime = SDL_GetTicks();
                fl = 0;
            }
            if (prejump.type == SDL_MOUSEBUTTONDOWN && prejump.button.x > 900 && prejump.button.x < 985 && prejump.button.y > 15 && prejump.button.y < 55)
            {
                moftplayer(10);
                SDL_RenderPresent(renderer);
                return (distance - delta_dir[direction]) * 10;
            }
            if (prejump.type == SDL_QUIT || (prejump.type == SDL_KEYDOWN && prejump.key.keysym.sym == SDLK_ESCAPE))
            {
                exit(0);
            }
        }
        state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_SPACE])
        {
            moftplayer(((SDL_GetTicks() - starttime) / sensitivity > MAXENERGY ? MAXENERGY : (SDL_GetTicks() - starttime) / sensitivity) / 30);
            dp_energy(SDL_GetTicks() - starttime);
            SDL_RenderPresent(renderer);
        }
    }
    energy = endtime - starttime;
    return energy / sensitivity > 0 ? energy / sensitivity : 1; // tuning, to make the press time reasonable, or not, if you wish?
}

int check_cur(int energy)
/*check wether the jump leaves the current platform*/
{
    switch (cur_type)
    {
    case 0:
    case 1:
    case 2:
        if ((landing == 3 && direction == 0 && landing == 3 && delta_dir[0] + energy / 10.0 <= -7.5 + cur_size * 3.5 / 100.0) || (direction == 1 && delta_dir[1] + energy / 10.0 <= -6 + cur_size * 3.0 / 100.0))
        {
            hop_and_update(energy);
            status--;
            if (status >= 1)
            {
                dp_parachute(0, direction, cur_size, 0);
                SDL_RenderPresent(renderer);
                return 1;
            }
            else
            {
                landing = 4;
                land();
                Mix_FadeOutMusic(300);
                if_record();
                menu();
            }
        }
        else if ((direction == 0 && delta_dir[0] + energy / 10.0 > -7.5 + cur_size * 3.5 / 100.0 && delta_dir[0] + energy / 10.0 < 9 - cur_size * 3.0 / 100.0) || (direction == 1 && delta_dir[1] + energy / 10.0 > -6 + cur_size * 3.0 / 100.0 && delta_dir[1] + energy / 10.0 < 7 - cur_size * 2.0 / 100.0))
        {
            hop_and_update(energy);
            if (landing == 3)
            {
                score += 1;
                landing = 0;
            }
            return 1;
        }
        else
            return 0;
        break;
    case 3:
    case 4:
        if ((landing == 3 && direction == 0 && delta_dir[0] + energy / 10.0 <= -10.0 + cur_size * 4.5 / 100.0) || (landing == 3 && direction == 1 && delta_dir[1] + energy / 10.0 <= -7 + cur_size * 3.5 / 100.0))
        {
            hop_and_update(energy);
            status--;
            if (status >= 1)
            {
                dp_parachute(0, direction, cur_size, 0);
                SDL_RenderPresent(renderer);
                return 1;
            }
            else
            {
                landing = 4;
                land();
                Mix_FadeOutMusic(300);
                if_record();
                menu();
            }
        }
        else if ((direction == 0 && delta_dir[0] + energy / 10.0 > -10 + cur_size * 4.5 / 100.0 && delta_dir[0] + energy / 10.0 < 11 - cur_size * 4.0 / 100.0) || (direction == 1 && delta_dir[1] + energy / 10.0 > -7 + cur_size * 3.5 / 100.0 && delta_dir[1] + energy / 10.0 < 9 - cur_size * 3.0 / 100.0))
        {
            hop_and_update(energy);
            if (landing == 3)
            {
                score += 1;
                landing = 0;
            }
            return 1;
        }
        else
            return 0;
        break;
    default:
        break;
    }
    return 0;
}

void hop_and_update(int energy)
/*jump within the current platform*/
{
    jump(energy, 0);
    delta_dir[direction] += energy / 10.0;
    delta_dir[1 - direction] = 0;
}

int check_nxt()
/*check the landing status, and decide the landing mode*/
{
    switch (nxt_type)
    {
    case 0:
    case 1:
    case 2:
        if (delta_dir_temp[0] > -7.5 + nxt_size * 3.5 / 100.0 && delta_dir_temp[0] < 9 - nxt_size * 3.0 / 100.0 && delta_dir_temp[1] > -6 + nxt_size * 3.0 / 100.0 && delta_dir_temp[1] < 7 - nxt_size * 2.0 / 100.0)
        {
            if (delta_dir_temp[0] > -2 && delta_dir_temp[0] < 2 && delta_dir_temp[1] > -2 && delta_dir_temp[1] < 2)
            {
                score_temp += 2;
                if (nxt_type == 1)    // a perfect landing on platform gives you a parachute
                    status_temp += 1; // delays the refresh of parachute number until after landing
            }
            else
                score_temp += 1;
            landing = 0;
            return 0;
        }
        else
        {
            if (status >= 2)
                landing = 3;
            else if (delta_dir_temp[0] > 0 || delta_dir_temp[1] > 0)
                landing = 1;
            else if (delta_dir_temp[0] < 0 || delta_dir_temp[1] < 0)
                landing = 2;
            return -1;
        }
        break;
    case 3:
    case 4:
        if (delta_dir_temp[0] > -10 + nxt_size * 4.5 / 100.0 && delta_dir_temp[0] < 11 - nxt_size * 4.0 / 100.0 && delta_dir_temp[1] > -7 + nxt_size * 3.5 / 100.0 && delta_dir_temp[1] < 9 - nxt_size * 3.0 / 100.0)
        {
            if (nxt_type == 3) // platform no.3 is the bonus platform
                score_temp += 1;
            if (delta_dir_temp[0] > -2 && delta_dir_temp[0] < 2 && delta_dir_temp[1] > -2 && delta_dir_temp[1] < 2)
                score_temp += 2;
            else
                score_temp += 1;
            landing = 0;
            return 0;
        }
        else
        {
            if (status >= 2)
                landing = 3;
            else if (delta_dir_temp[0] > 0 || delta_dir_temp[1] > 0)
                landing = 1;
            else if (delta_dir_temp[0] < 0 || delta_dir_temp[1] < 0)
                landing = 2;
            return -1;
        }
        break;
    default:
        break;
    }
    return -1;
}

void jump(int energy, int mode)
/*the jumping process
 *parameter mode refers to whether the jump is between platforms or within one platform;
 *when mode==1, the jump is between platforms; when mode==0,  the jump is within one platform*/
{
    double i = 0.0;
    int num = energy / 10;
    double step = energy * 1.0 / num; // each step roughly 10 in energy
    Mix_PlayChannel(-1, jump_au, 0);
    while ((energy - i) > -1.0e-6)
    {
        // keep the stage
        dp_background();
        dp_platform(distance, direction, nxt_type, nxt_size, 0, 0);
        dp_platform(0, direction, cur_type, cur_size, 0, 0);
        //  move the ball
        dp_player_jump(i, direction, mode, energy);
        SDL_RenderPresent(renderer);
        i += step;
        SDL_Delay(15);
    }
    if ((nxt_type == 3 && score_temp == 3) || (nxt_type != 3 && score_temp == 2))
        Mix_PlayChannel(-1, perfect_au, 0);
}

void update_location()
{
    delta_dir[0] = delta_dir_temp[0];
    delta_dir_temp[0] = 0;
    delta_dir[1] = delta_dir_temp[1];
    delta_dir_temp[0] = 0;
}

void land()
/*display different landing themes accordingly*/
{
    int h = 0;
    switch (landing)
    {
    case 0:
        break;
    case 1: // fall behind the platforms
        Mix_PlayChannel(-1, fall_au, 0);
        while (h < 100 - nxt_size * 0.3)
        {
            dp_background();
            dp_player(distance, direction, nxt_size, h);
            dp_platform(distance, direction, nxt_type, nxt_size, 0, 0);
            dp_platform(0, direction, cur_type, cur_size, 0, 0);
            SDL_RenderPresent(renderer);
            SDL_Delay(15);
            h += 5;
        }
        break;
    case 2: // fall between the platforms
        Mix_PlayChannel(-1, fall_au, 0);
        while (h < 100 - nxt_size * 0.3)
        {
            dp_background();
            dp_platform(distance, direction, nxt_type, nxt_size, 0, 0);
            dp_player(distance, direction, nxt_size, h);
            dp_platform(0, direction, cur_type, cur_size, 0, 0);
            SDL_RenderPresent(renderer);
            SDL_Delay(15);
            h += 5;
        }
        break;
    case 3: // show parachute
        dp_parachute(distance, direction, nxt_size, 0);
        SDL_RenderPresent(renderer);
        break;
    case 4: // fall before the platforms, for hop cases
        Mix_PlayChannel(-1, fall_au, 0);
        while (h < 100 - nxt_size * 0.3)
        {
            dp_background();
            dp_platform(distance, direction, nxt_type, nxt_size, 0, 0);
            dp_platform(0, direction, cur_type, cur_size, 0, 0);
            dp_player(0, direction, cur_size, h);
            SDL_RenderPresent(renderer);
            SDL_Delay(15);
            h += 5;
        }
        break;
    default:
        break;
    }
}

void update_status()
{
    status += status_temp;
    status_temp = 0;
    score += score_temp;
    score_temp = 0;
    SDL_Delay(300);
}

void if_record()
/*show different ending themes for different scores(whether or not the record is broken)*/
{
    if (score > record)
    {
        record = score;
        SDL_SetRenderDrawColor(renderer, 0xf0, 0xff, 0xff, 0xff);
        SDL_RenderClear(renderer);

        // show fireworks
        SDL_Surface *newrecord_surface = IMG_Load("./res/fireworks.png");
        SDL_Texture *newrecord_texture = SDL_CreateTextureFromSurface(renderer, newrecord_surface);
        SDL_Rect rect_newrecord = {150, 20, 320, 580};
        SDL_RenderCopy(renderer, newrecord_texture, NULL, &rect_newrecord);
        SDL_DestroyTexture(newrecord_texture);
        SDL_FreeSurface(newrecord_surface);

        // show "new record"
        comic = TTF_OpenFont("./res/comic.ttf", 50);
        SDL_Color black = {0x00, 0x00, 0x00, 0xff};
        SDL_Surface *title_surface = TTF_RenderText_Blended(comic, "New Record !", black);
        SDL_Texture *title_texture = SDL_CreateTextureFromSurface(renderer, title_surface);
        SDL_Rect rect_title = {500, 180, title_surface->w, title_surface->h};
        SDL_RenderCopy(renderer, title_texture, NULL, &rect_title);
        SDL_DestroyTexture(title_texture);
        SDL_FreeSurface(title_surface);

        // show record
        char recordchar[15];
        sprintf(recordchar, "%d", record);
        SDL_Surface *record_surface = TTF_RenderText_Blended(comic, recordchar, black);
        SDL_Texture *record_texture = SDL_CreateTextureFromSurface(renderer, record_surface);
        SDL_Rect rect_record = {590, 300, record_surface->w, record_surface->h};
        SDL_RenderCopy(renderer, record_texture, NULL, &rect_record);
        SDL_DestroyTexture(record_texture);
        SDL_FreeSurface(record_surface);
        TTF_CloseFont(comic);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 0xf0, 0xff, 0xff, 0xff);
        SDL_RenderClear(renderer);
        SDL_Surface *end_surface = IMG_Load("./res/rain.png");
        SDL_Texture *end_texture = SDL_CreateTextureFromSurface(renderer, end_surface);
        SDL_Rect rect_end = {130, 60, 340, 400};
        SDL_RenderCopy(renderer, end_texture, NULL, &rect_end);
        SDL_DestroyTexture(end_texture);
        SDL_FreeSurface(end_surface);
        comic = TTF_OpenFont("./res/comic.ttf", 50);
        SDL_Color black = {0x00, 0x00, 0x00, 0xff};
        char curscorechar[15];
        sprintf(curscorechar, "%d", score);
        SDL_Surface *title_surface = TTF_RenderText_Blended(comic, "Jump Missed", black);
        SDL_Texture *title_texture = SDL_CreateTextureFromSurface(renderer, title_surface);
        SDL_Rect rect_title = {500, 180, title_surface->w, title_surface->h};
        SDL_RenderCopy(renderer, title_texture, NULL, &rect_title);
        SDL_DestroyTexture(title_texture);
        SDL_FreeSurface(title_surface);
        // show score
        SDL_Surface *curscore_surface = TTF_RenderText_Blended(comic, curscorechar, black);
        SDL_Texture *curscore_texture = SDL_CreateTextureFromSurface(renderer, curscore_surface);
        SDL_Rect rect_curscore = {590, 300, curscore_surface->w, curscore_surface->h};
        SDL_RenderCopy(renderer, curscore_texture, NULL, &rect_curscore);
        SDL_DestroyTexture(curscore_texture);
        SDL_FreeSurface(curscore_surface);
        TTF_CloseFont(comic);
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
    }
}

void move()
/*moves the player and current platform to center page for next jump*/
{

    double dirplayer = distance;
    double dirplatform2 = distance;
    double dirplatform1 = 0;
    double step = 3.0;
    while (dirplayer != 0 || dirplatform2 != 0)
    {
        dirplayer = (dirplayer - step) > 0 ? (dirplayer - step) : 0;
        dirplatform2 = (dirplatform2 - step) > 0 ? (dirplatform2 - step) : 0;
        dirplatform1 -= step;
        cur_size += 35;
        dp_background();
        dp_platform(dirplatform2, direction, nxt_type, nxt_size, 0, 0);
        if (direction == 0)
            dp_platform(dirplatform1, direction, cur_type, cur_size, 0, cur_size);
        else
            dp_platform(dirplatform1, direction, cur_type, cur_size, cur_size, cur_size);
        dp_player(dirplayer, direction, nxt_size, 0);
        if (landing == 3)
            dp_parachute(dirplayer, direction, nxt_size, 0);
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
    cur_type = nxt_type;
    cur_size = nxt_size;
}

void next()
{
    dp_background();
    gen_platform();
    dp_platform(0, direction, cur_type, cur_size, 0, 0);
    dp_player(0, direction, cur_size, 0);
    if (landing == 3)
        dp_parachute(0, direction, cur_size, 0);
    SDL_RenderPresent(renderer);
}

void dp_energy(int amount)
/*demostrate the amount of energy built up via the length of a bar*/
{
    SDL_SetRenderDrawColor(renderer, 0xcd, 0x5c, 0x5c, 0xff);
    int t = amount < MAXENERGY * sensitivity ? amount : MAXENERGY * sensitivity;
    SDL_Rect rect_energy = {370, 25, t / sensitivity / 3, 50};
    SDL_RenderFillRect(renderer, &rect_energy);

    comic = TTF_OpenFont("./res/comic.ttf", 40);
    SDL_Color turquoise = {0x66, 0x8b, 0x8b, 0xff};
    SDL_Surface *title1_surface = TTF_RenderText_Blended(comic, "energy", turquoise);
    SDL_Texture *title1_texture = SDL_CreateTextureFromSurface(renderer, title1_surface);
    SDL_Rect rect_title1 = {230, 20, title1_surface->w, title1_surface->h};
    SDL_RenderCopy(renderer, title1_texture, NULL, &rect_title1);
    SDL_DestroyTexture(title1_texture);
    SDL_FreeSurface(title1_surface);
    TTF_CloseFont(comic);
}

void moftplayer(int level)
/*demostrate the build up of energy via changing the shape of the character*/
{
    dp_background();
    dp_platform(distance, direction, nxt_type, nxt_size, 0, 0);
    dp_platform(0, direction, cur_type, cur_size, 0, 0);
    SDL_FRect rect_player = {(ORIXPLAYER + delta_dir[0] * 10.0 - delta_dir[1] * 6.0) - cur_size / 2.0, (ORIYPLAYER - delta_dir[0] * 1.0 - delta_dir[1] * 3.0) - cur_size * 0.22 / 2.0 + level, 50, 50 - level};
    SDL_RenderCopyF(renderer, player_texture, NULL, &rect_player);
    if (landing == 3)
        dp_parachute(0, direction, cur_size, level);
}

void dp_menu(int a, int b)
/*display the menu page;
 *parameter a and b indicate the three states of the page
 *when a is set to one, Settings pops out; when b is set to one, Readme pops out.*/
{
    SDL_SetRenderDrawColor(renderer, 0xf0, 0xff, 0xff, 0xff);
    SDL_RenderClear(renderer);
    SDL_Color black = {0x00, 0x00, 0x00, 0xff};

    // the settings page, on the right,
    SDL_Surface *settings_surface = IMG_Load("./res/sidepage.png");
    SDL_Texture *settings_texture = SDL_CreateTextureFromSurface(renderer, settings_surface);
    SDL_SetTextureAlphaMod(settings_texture, 128 + 127 * a);
    SDL_Rect rect_settings = {470 + 100 * a, 150 - 50 * a, 300 + 100 * a, 300 + 100 * a};
    SDL_RenderCopy(renderer, settings_texture, NULL, &rect_settings);
    SDL_DestroyTexture(settings_texture);
    SDL_FreeSurface(settings_surface);
    comic = TTF_OpenFont("./res/comic.ttf", 16 * (1 + a));
    SDL_Surface *title1_surface = TTF_RenderText_Blended(comic, "Settings", black);
    SDL_Texture *title1_texture = SDL_CreateTextureFromSurface(renderer, title1_surface);
    SDL_SetTextureAlphaMod(title1_texture, 192 + 63 * a);
    SDL_Rect rect_title1 = {700 + 10 * a, 285 - 170 * a, title1_surface->w, title1_surface->h};
    SDL_RenderCopy(renderer, title1_texture, NULL, &rect_title1);
    SDL_DestroyTexture(title1_texture);
    SDL_FreeSurface(title1_surface);
    TTF_CloseFont(comic);

    // the readme page ,on the left
    SDL_Surface *readme_surface = IMG_Load("./res/sidepage.png");
    SDL_Texture *readme_texture = SDL_CreateTextureFromSurface(renderer, readme_surface);
    SDL_SetTextureAlphaMod(readme_texture, 128 + 127 * b);
    SDL_Rect rect_readme = {230 - 200 * b, 150 - 50 * b, 300 + 100 * b, 300 + 100 * b};
    SDL_RenderCopy(renderer, readme_texture, NULL, &rect_readme);
    SDL_DestroyTexture(readme_texture);
    SDL_FreeSurface(readme_surface);
    comic = TTF_OpenFont("./res/comic.ttf", 16 * (1 + b));
    SDL_Surface *title2_surface = TTF_RenderText_Blended(comic, "Readme", black);
    SDL_Texture *title2_texture = SDL_CreateTextureFromSurface(renderer, title2_surface);
    SDL_SetTextureAlphaMod(title2_texture, 192 + 63 * b);
    SDL_Rect rect_title2 = {240 - 70 * b, 285 - 170 * b, title2_surface->w, title2_surface->h};
    SDL_RenderCopy(renderer, title2_texture, NULL, &rect_title2);
    SDL_DestroyTexture(title2_texture);
    SDL_FreeSurface(title2_surface);
    TTF_CloseFont(comic);

    // the main page
    SDL_Surface *menupage_surface;
    switch (status)
    {
    case 1:
        menupage_surface = IMG_Load("./res/startpage.png");
        break;
    case 0:
        menupage_surface = IMG_Load("./res/restartpage.png");
        break;
    }
    SDL_Texture *menupage_texture = SDL_CreateTextureFromSurface(renderer, menupage_surface);
    SDL_Rect rect_menu = {300, 100, 400, 400};
    SDL_RenderCopy(renderer, menupage_texture, NULL, &rect_menu);
    SDL_DestroyTexture(menupage_texture);
    SDL_FreeSurface(menupage_surface);
}

void dp_settings()
/*display the settings page*/
{
    comic = TTF_OpenFont("./res/comic.ttf", 20);
    SDL_Color black = {0x00, 0x00, 0x00, 0xff};
    SDL_Surface *title1_surface = TTF_RenderText_Blended(comic, "Sensitivity", black);
    SDL_Texture *title1_texture = SDL_CreateTextureFromSurface(renderer, title1_surface);
    SDL_Rect rect_title1 = {700, 160, title1_surface->w, title1_surface->h};
    SDL_RenderCopy(renderer, title1_texture, NULL, &rect_title1);
    SDL_DestroyTexture(title1_texture);
    SDL_FreeSurface(title1_surface);
    TTF_CloseFont(comic);

    comic = TTF_OpenFont("./res/comic.ttf", 12);
    SDL_Surface *title2_surface = TTF_RenderText_Blended(comic, "the smaller, the more sensitive to press", black);
    SDL_Texture *title2_texture = SDL_CreateTextureFromSurface(renderer, title2_surface);
    SDL_Rect rect_title2 = {705, 340, title2_surface->w, title2_surface->h};
    SDL_RenderCopy(renderer, title2_texture, NULL, &rect_title2);
    SDL_DestroyTexture(title2_texture);
    SDL_FreeSurface(title2_surface);
    TTF_CloseFont(comic);

    SDL_Rect rect_buttons = {720, 190, 150, 150};
    SDL_Surface *buttons_surface = IMG_Load("./res/buttons.png");
    SDL_Texture *buttons_texture = SDL_CreateTextureFromSurface(renderer, buttons_surface);
    SDL_RenderCopy(renderer, buttons_texture, NULL, &rect_buttons);

    SDL_DestroyTexture(buttons_texture);
    SDL_FreeSurface(buttons_surface);
    SDL_Rect rect_check = {720, 165 + 30 * sensitivity, 20, 20};
    SDL_Surface *check_surface = IMG_Load("./res/check.png");
    SDL_Texture *check_texture = SDL_CreateTextureFromSurface(renderer, check_surface);
    SDL_RenderCopy(renderer, check_texture, NULL, &rect_check);
    SDL_DestroyTexture(check_texture);
    SDL_FreeSurface(check_surface);

    SDL_Rect rect_done = {745, 435, 60, 30};
    SDL_Surface *done_surface = IMG_Load("./res/button.png");
    SDL_Texture *done_texture = SDL_CreateTextureFromSurface(renderer, done_surface);
    SDL_RenderCopy(renderer, done_texture, NULL, &rect_done);
    SDL_DestroyTexture(done_texture);
    SDL_FreeSurface(done_surface);

    comic = TTF_OpenFont("./res/comic.ttf", 18);
    SDL_Surface *title3_surface = TTF_RenderText_Blended(comic, "DONE", black);
    SDL_Texture *title3_texture = SDL_CreateTextureFromSurface(renderer, title3_surface);
    SDL_Rect rect_title3 = {748, 435, title3_surface->w, title3_surface->h};
    SDL_RenderCopy(renderer, title3_texture, NULL, &rect_title3);
    SDL_DestroyTexture(title3_texture);
    SDL_FreeSurface(title3_surface);
    TTF_CloseFont(comic);
}

void dp_readme(int pg)
/*display the game intro page
 *parameter pg specifies the page to show*/
{
    SDL_Color black = {0x00, 0x00, 0x00, 0xff};
    if (pg == 1)
    {
        comic = TTF_OpenFont("./res/comic.ttf", 22);
        SDL_Surface *title_read_surface = TTF_RenderText_Blended(comic, "Intro", black);
        SDL_Texture *title_read_texture = SDL_CreateTextureFromSurface(renderer, title_read_surface);
        SDL_Rect rect_title_read = {100, 180, title_read_surface->w, title_read_surface->h};
        SDL_RenderCopy(renderer, title_read_texture, NULL, &rect_title_read);
        TTF_CloseFont(comic);
        comic = TTF_OpenFont("./res/comic.ttf", 16);
        SDL_Surface *read_surface = TTF_RenderText_Blended_Wrapped(comic, "Player, be ware.\nWhat you see is but the projection of a three dimensional world, and the platforms are round and square from above.", black, 200);
        SDL_Texture *read_texture = SDL_CreateTextureFromSurface(renderer, read_surface);
        SDL_Rect rect_read = {110, 230, read_surface->w, read_surface->h};
        SDL_RenderCopy(renderer, read_texture, NULL, &rect_read);
        SDL_DestroyTexture(read_texture);
        SDL_FreeSurface(read_surface);
        SDL_DestroyTexture(title_read_texture);
        SDL_FreeSurface(title_read_surface);
        TTF_CloseFont(comic);
    }
    else if (pg == 2)
    {
        comic = TTF_OpenFont("./res/comic.ttf", 22);
        SDL_Surface *title_read_surface = TTF_RenderText_Blended(comic, "How", black);
        SDL_Texture *title_read_texture = SDL_CreateTextureFromSurface(renderer, title_read_surface);
        SDL_Rect rect_title_read = {100, 180, title_read_surface->w, title_read_surface->h};
        SDL_RenderCopy(renderer, title_read_texture, NULL, &rect_title_read);
        TTF_CloseFont(comic);
        comic = TTF_OpenFont("./res/comic.ttf", 16);
        SDL_Surface *read_surface = TTF_RenderText_Blended_Wrapped(comic, "Press space to JUMP\n\nClick auto to CHEAT", black, 200);
        SDL_Texture *read_texture = SDL_CreateTextureFromSurface(renderer, read_surface);
        SDL_Rect rect_read = {110, 230, read_surface->w, read_surface->h};
        SDL_RenderCopy(renderer, read_texture, NULL, &rect_read);
        SDL_DestroyTexture(read_texture);
        SDL_FreeSurface(read_surface);
        SDL_DestroyTexture(title_read_texture);
        SDL_FreeSurface(title_read_surface);
        TTF_CloseFont(comic);
    }
    else if (pg == 3)
    {
        comic = TTF_OpenFont("./res/comic.ttf", 22);
        SDL_Surface *title_read_surface = TTF_RenderText_Blended(comic, "By the way", black);
        SDL_Texture *title_read_texture = SDL_CreateTextureFromSurface(renderer, title_read_surface);
        SDL_Rect rect_title_read = {100, 180, title_read_surface->w, title_read_surface->h};
        SDL_RenderCopy(renderer, title_read_texture, NULL, &rect_title_read);
        TTF_CloseFont(comic);
        comic = TTF_OpenFont("./res/comic.ttf", 16);
        SDL_Surface *read_surface = TTF_RenderText_Blended_Wrapped(comic, "Landing perfectly is one extra point\nLanding on \"star\" is one extra point\nLanding on \"planet\" is one parachute, which can save you once", black, 200);
        SDL_Texture *read_texture = SDL_CreateTextureFromSurface(renderer, read_surface);
        SDL_Rect rect_read = {110, 230, read_surface->w, read_surface->h};
        SDL_RenderCopy(renderer, read_texture, NULL, &rect_read);
        SDL_DestroyTexture(read_texture);
        SDL_FreeSurface(read_surface);
        SDL_DestroyTexture(title_read_texture);
        SDL_FreeSurface(title_read_surface);
        TTF_CloseFont(comic);
    }
    SDL_Rect rect_done = {205, 435, 60, 30};
    SDL_Surface *done_surface = IMG_Load("./res/button.png");
    SDL_Texture *done_texture = SDL_CreateTextureFromSurface(renderer, done_surface);
    SDL_RenderCopy(renderer, done_texture, NULL, &rect_done);
    SDL_DestroyTexture(done_texture);
    SDL_FreeSurface(done_surface);
    comic = TTF_OpenFont("./res/comic.ttf", 18);
    if (pg == 3)
    {
        SDL_Surface *title3_surface = TTF_RenderText_Blended(comic, "DONE", black);
        SDL_Texture *title3_texture = SDL_CreateTextureFromSurface(renderer, title3_surface);
        SDL_Rect rect_title3 = {208, 435, title3_surface->w, title3_surface->h};
        SDL_RenderCopy(renderer, title3_texture, NULL, &rect_title3);
        SDL_DestroyTexture(title3_texture);
        SDL_FreeSurface(title3_surface);
    }
    else
    {
        SDL_Surface *title3_surface = TTF_RenderText_Blended(comic, "NEXT", black);
        SDL_Texture *title3_texture = SDL_CreateTextureFromSurface(renderer, title3_surface);
        SDL_Rect rect_title3 = {208, 435, title3_surface->w, title3_surface->h};
        SDL_RenderCopy(renderer, title3_texture, NULL, &rect_title3);
        SDL_DestroyTexture(title3_texture);
        SDL_FreeSurface(title3_surface);
    }
    TTF_CloseFont(comic);
}

void dp_background()
/*refresh the background, with auto button,parachute number, score board, score, record*/
{
    // sets background color
    SDL_SetRenderDrawColor(renderer, 0xf0, 0xff, 0xff, 0xff);
    SDL_RenderClear(renderer);

    // score board
    SDL_Rect rect_scoreboard = {8, 8, 200, 120};
    SDL_RenderCopy(renderer, scoreboard_texture, NULL, &rect_scoreboard);
    comic = TTF_OpenFont("./res/comic.ttf", 30);
    SDL_Color turquoise = {0x66, 0x8b, 0x8b, 0xff};
    // current score
    SDL_Surface *title_score_surface = TTF_RenderText_Blended(comic, "Score", turquoise);
    SDL_Texture *title_score_texture = SDL_CreateTextureFromSurface(renderer, title_score_surface);
    SDL_Rect rect_title_score = {30, 20, title_score_surface->w, title_score_surface->h};
    SDL_RenderCopy(renderer, title_score_texture, NULL, &rect_title_score);
    SDL_DestroyTexture(title_score_texture);
    SDL_FreeSurface(title_score_surface);
    char curscorechar[15];
    sprintf(curscorechar, "%d", score);
    SDL_Surface *curscore_surface = TTF_RenderText_Blended(comic, curscorechar, turquoise);
    SDL_Texture *curscore_texture = SDL_CreateTextureFromSurface(renderer, curscore_surface);
    SDL_Rect rect_curscore = {145, 20, curscore_surface->w, curscore_surface->h};
    SDL_RenderCopy(renderer, curscore_texture, NULL, &rect_curscore);
    SDL_DestroyTexture(curscore_texture);
    SDL_FreeSurface(curscore_surface);
    // record
    SDL_Surface *title_record_surface = TTF_RenderText_Blended(comic, "Record", turquoise);
    SDL_Texture *title_record_texture = SDL_CreateTextureFromSurface(renderer, title_record_surface);
    SDL_Rect rect_title_record = {30, 60, title_record_surface->w, title_record_surface->h};
    SDL_RenderCopy(renderer, title_record_texture, NULL, &rect_title_record);
    SDL_DestroyTexture(title_record_texture);
    SDL_FreeSurface(title_record_surface);
    char recordchar[15];
    sprintf(recordchar, "%d", record);
    SDL_Surface *record_surface = TTF_RenderText_Blended(comic, recordchar, turquoise);
    SDL_Texture *record_texture = SDL_CreateTextureFromSurface(renderer, record_surface);
    SDL_Rect rect_record = {145, 60, record_surface->w, record_surface->h};
    SDL_RenderCopy(renderer, record_texture, NULL, &rect_record);
    SDL_DestroyTexture(record_texture);
    SDL_FreeSurface(record_surface);
    TTF_CloseFont(comic);

    // auto, currently unlimited usage, for TA's benefit.
    SDL_Rect rect_button_auto = {900, 15, 85, 40};
    SDL_RenderCopy(renderer, button_auto_texture, NULL, &rect_button_auto);
    comic = TTF_OpenFont("./res/comic.ttf", 23);
    SDL_Surface *title1_surface = TTF_RenderText_Blended(comic, "Auto", turquoise);
    SDL_Texture *title1_texture = SDL_CreateTextureFromSurface(renderer, title1_surface);
    SDL_Rect rect_title1 = {915, 17, title1_surface->w, title1_surface->h};
    SDL_RenderCopy(renderer, title1_texture, NULL, &rect_title1);
    SDL_DestroyTexture(title1_texture);
    SDL_FreeSurface(title1_surface);

    // parachute
    SDL_Rect rect_parachute = {900, 65, 30, 25};
    SDL_RenderCopy(renderer, parachute_texture, NULL, &rect_parachute);
    char parachute_char[15];
    sprintf(parachute_char, "%d", status - 1 > 0 ? status - 1 : 0);
    SDL_Surface *parachute_num_surface = TTF_RenderText_Blended(comic, parachute_char, turquoise);
    SDL_Texture *parachute_num_texture = SDL_CreateTextureFromSurface(renderer, parachute_num_surface);
    SDL_Rect rect_parachute_num = {940, 60, parachute_num_surface->w, parachute_num_surface->h};
    SDL_RenderCopy(renderer, parachute_num_texture, NULL, &rect_parachute_num);
    SDL_DestroyTexture(parachute_num_texture);
    SDL_FreeSurface(parachute_num_surface);
    TTF_CloseFont(comic);
}

void dp_platform(int loc, int dir, int type, int size, int x_move, int y_move)
/*display a platform
 *size is actually the reduced size
 *x_move and y_move is used when moving the platform off stage to achieve the effect that they get smaller and vanish*/
{
    if (dir == 0)
    {
        SDL_FRect rect_platform = {ORIXPLATFORM + loc * 10.0 + x_move, ORIYPLATFORM - loc * 1.0 + y_move, 250 - size, 150 - size * 0.6};
        SDL_RenderCopyF(renderer, platform_texture[type], NULL, &rect_platform);
    }
    else
    {
        SDL_FRect rect_platform = {ORIXPLATFORM - loc * 6.0 + x_move, ORIYPLATFORM - loc * 3.0 + y_move, 250 - size, 150 - size * 0.6};
        SDL_RenderCopyF(renderer, platform_texture[type], NULL, &rect_platform);
    }
}

void dp_player(double loc, int dir, int size, int height)
/*display the player when on the surface
 *parameter size refers to the size of the platform it's on;
 *parameter height is used during moftplayer.*/
{
    if (dir == 0)
    {
        SDL_FRect rect_player = {(ORIXPLAYER + delta_dir[0] * 10.0 - delta_dir[1] * 6.0) + loc * 10.0 - size / 2.0, (ORIYPLAYER - delta_dir[0] * 1.0 - delta_dir[1] * 3.0) - loc * 1.0 - size * 0.22 / 2.0 + height, 50, 50};
        SDL_RenderCopyF(renderer, player_texture, NULL, &rect_player);
    }
    else
    {
        SDL_FRect rect_player = {(ORIXPLAYER + delta_dir[0] * 10.0 - delta_dir[1] * 6.0) - loc * 6.0 - size / 2.0, (ORIYPLAYER - delta_dir[0] * 1.0 - delta_dir[1] * 3.0) - loc * 3.0 - size * 0.22 / 2.0 + height, 50, 50};
        SDL_RenderCopyF(renderer, player_texture, NULL, &rect_player);
    }
}

void dp_player_jump(double i, int dir, int mode, int energy)
/* display the player during jump. jumping in dir[0] removes delta in dir[1], and vis versa.
 * mode==1 for jumping between platforms; mode==0 for hopping within one platform.
 *The height of hop is raised for better visual effect.*/
{
    double x, y;
    if (dir == 0)
    {
        x = (ORIXPLAYER + delta_dir[0] * 10.0 - delta_dir[1] * 6.0) + delta_dir[1] * 6.0 * i / energy - cur_size / 2.0 - mode * (nxt_size - cur_size) * 1.0 * i / energy / 2.0 + i * 10.0 / 10.0;
        y = (ORIYPLAYER - delta_dir[0] * 1.0 - delta_dir[1] * 3.0) + delta_dir[1] * 3.0 * i / energy - cur_size * 0.22 / 2.0 - mode * (nxt_size - cur_size) * 0.22 * i / energy / 2.0 - i * 1.0 / 10.0 - (1 + 500 / energy * (1 - mode)) * (energy * energy / 4.0 - (i - energy / 2.0) * (i - energy / 2.0)) / 200;
    }
    else if (dir == 1)
    {
        x = (ORIXPLAYER + delta_dir[0] * 10.0 - delta_dir[1] * 6.0) - delta_dir[0] * 10.0 * i / energy - cur_size / 2.0 - mode * (nxt_size - cur_size) * 1.0 * i / energy / 2.0 - i * 6.0 / 10.0;
        y = (ORIYPLAYER - delta_dir[0] * 1.0 - delta_dir[1] * 3.0) + delta_dir[0] * 1.0 * i / energy - cur_size * 0.22 / 2.0 - mode * (nxt_size - cur_size) * 0.22 * i / energy / 2.0 - i * 3.0 / 10.0 - (1 + 500 / energy * (1 - mode)) * (energy * energy / 4.0 - (i - energy / 2.0) * (i - energy / 2.0)) / 200;
    }
    SDL_FRect rect_player = {x, y, 50, 50};
    SDL_RenderCopyF(renderer, player_texture, NULL, &rect_player);
}

void dp_parachute(double loc, int dir, int size, int height)
/*show parachute, which save the character from falling
 *parameter height is used during moftplayer*/
{
    if (dir == 0)
    {
        SDL_FRect rect_parachute = {(ORIXPLAYER + delta_dir[0] * 10.0 - delta_dir[1] * 6.0) + loc * 10.0 - size / 2.0 - 6, (ORIYPLAYER - delta_dir[0] * 1.0 - delta_dir[1] * 3.0) - loc * 1.0 - size * 0.22 / 2.0 - 45 + height, 60, 50};
        SDL_RenderCopyF(renderer, parachute_texture, NULL, &rect_parachute);
    }
    else if (dir == 1)
    {
        SDL_FRect rect_parachute = {(ORIXPLAYER + delta_dir[0] * 10.0 - delta_dir[1] * 6.0) - loc * 6.0 - size / 2.0 - 6, (ORIYPLAYER - delta_dir[0] * 1.0 - delta_dir[1] * 3.0) - loc * 3.0 - size * 0.22 / 2.0 - 45 + height, 60, 50};
        SDL_RenderCopyF(renderer, parachute_texture, NULL, &rect_parachute);
    }
}

void init()
{
    srand(time(NULL));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(IMG_INIT_PNG);
    SDL_Init(IMG_INIT_JPG);
    TTF_Init();
    Mix_Init(MIX_INIT_OGG);
    Mix_Init(MIX_INIT_MP3);
    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);
    bgm = Mix_LoadMUS("./res/bgm.mp3");
    perfect_au = Mix_LoadWAV("./res/perfect_audio.ogg");
    jump_au = Mix_LoadWAV("./res/jump_audio.ogg");
    fall_au = Mix_LoadWAV("./res/fall_audio.ogg");
    click_au = Mix_LoadWAV("./res/click_audio.ogg");
    Mix_VolumeMusic(32);
    Mix_VolumeChunk(jump_au, 64);
    Mix_VolumeChunk(perfect_au, 15);
    Mix_VolumeChunk(fall_au, 30);
    Mix_VolumeChunk(click_au, 10);
    window = SDL_CreateWindow("Jump", 100, 100, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    player_surface = IMG_Load("./res/neptune.png");
    player_texture = SDL_CreateTextureFromSurface(renderer, player_surface);

    platform_surface[0] = IMG_Load("./res/cuboid1.png");
    platform_texture[0] = SDL_CreateTextureFromSurface(renderer, platform_surface[0]);
    platform_surface[1] = IMG_Load("./res/cuboid2.png");
    platform_texture[1] = SDL_CreateTextureFromSurface(renderer, platform_surface[1]);
    platform_surface[2] = IMG_Load("./res/cuboid3.png");
    platform_texture[2] = SDL_CreateTextureFromSurface(renderer, platform_surface[2]);
    platform_surface[3] = IMG_Load("./res/cylinder1.png");
    platform_texture[3] = SDL_CreateTextureFromSurface(renderer, platform_surface[3]);
    platform_surface[4] = IMG_Load("./res/cylinder2.png");
    platform_texture[4] = SDL_CreateTextureFromSurface(renderer, platform_surface[4]);
    scoreboard_surface = IMG_Load("./res/scoreboard.png");
    scoreboard_texture = SDL_CreateTextureFromSurface(renderer, scoreboard_surface);
    button_auto_surface = IMG_Load("./res/button.png");
    button_auto_texture = SDL_CreateTextureFromSurface(renderer, button_auto_surface);
    parachute_surface = IMG_Load("./res/parachute.png");
    parachute_texture = SDL_CreateTextureFromSurface(renderer, parachute_surface);
}

void shutdown()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    Mix_FreeChunk(jump_au);
    Mix_FreeChunk(perfect_au);
    Mix_FreeChunk(fall_au);
    Mix_FreeChunk(click_au);
    Mix_FreeMusic(bgm);
    Mix_CloseAudio();
    SDL_DestroyTexture(player_texture);
    SDL_FreeSurface(player_surface);
    for (int i = 0; i < NUM_PLATFORM; i++)
    {
        SDL_DestroyTexture(platform_texture[i]);
        SDL_FreeSurface(platform_surface[i]);
    }
    SDL_DestroyTexture(scoreboard_texture);
    SDL_FreeSurface(scoreboard_surface);
    SDL_DestroyTexture(button_auto_texture);
    SDL_FreeSurface(button_auto_surface);
    SDL_DestroyTexture(parachute_texture);
    SDL_FreeSurface(parachute_surface);

    IMG_Quit();
    SDL_Quit();
    TTF_Quit();
    Mix_Quit();
}