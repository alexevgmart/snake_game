#include "include.h"

void *direct(void){
	int tmp;
    bool flag = FALSE;

	while(TRUE){
		tmp = getch();
		if (tmp == 'w' || tmp == 'a' || tmp == 's' || tmp == 'd' || tmp == 'q'
			|| tmp == KEY_UP || tmp == KEY_LEFT || tmp == KEY_DOWN || tmp == KEY_RIGHT)
			flag = TRUE;

		if (!flag)
			continue;
		else {
			if ((user_packet.direction == 'w' && tmp == 's') || (user_packet.direction == KEY_UP && tmp == KEY_DOWN))
				continue;
			if ((user_packet.direction == 's' && tmp == 'w') || (user_packet.direction == KEY_DOWN && tmp == KEY_UP))
				continue;
			if ((user_packet.direction == 'a' && tmp == 'd') || (user_packet.direction == KEY_LEFT && tmp == KEY_RIGHT))
				continue;
			if ((user_packet.direction == 'd' && tmp == 'a') || (user_packet.direction == KEY_RIGHT && tmp == KEY_LEFT))
				continue;

			user_packet.direction = tmp;
			flag = FALSE;
		}
	}
}

void apple_cpy(void* get) {
	// if (*(bool*)get)
	// 	usleep(50000);
	// else
	// 	usleep(60000);
	// show_new_apple = TRUE;
	usleep(20000);
	user_packet.generated_apple = 0;
}

void wait_for_new_apple () {
	usleep(2500000);
	new_apple = TRUE;
}

void get_and_send_packets(void* server) {
	bool is_server = *(bool*)server;
	while (TRUE) {
		if (is_server) {
			packet tmp[4];
			memcpy(&tmp[0], &user_packet, sizeof(packet));
			for (int i = 1; i < users_alive + 1; i++)
				memcpy(&tmp[i], &clients[i - 1].packet, sizeof(packet));
			
			for (int i = 0; i < users_alive; i++)
				send(clients[i].sockfd, &tmp, sizeof(packet) * 4, 0);
			user_packet.generated_apple = 0;
			
			for (int i = 0; i < users_alive; i++)
				recv(clients[i].sockfd, &clients[i].packet, sizeof(packet), 0);
		}
		else {
			packet tmp[4];
			recv(cd, &tmp, sizeof(packet) * 4, 0);
			for (int i = 0; i < users_alive; i++) {
				for (int j = 0; j < 4; j++) {
					if (tmp[j].user_code == clients[i].client_code) {
						memcpy(&clients[i].packet, &tmp[j], sizeof(packet));
						break;
					}
				}
			}
			send(cd, &user_packet, sizeof(packet), 0);
			user_packet.generated_apple = 0;
		}

		for (int i = 0; i < users_alive; i++) {
			if (user_packet.apple.x != clients[i].packet.apple.x && user_packet.apple.y != clients[i].packet.apple.y && clients[i].packet.generated_apple == 1) {
				// mvaddch(user_packet.apple.y, user_packet.apple.y, ' ');
				// if (sent_apple) {
				// 	pthread_t pid;
				// 	// pthread_create(&pid, NULL, apple_cpy, &clients[i].packet.apple);
				// 	sent_apple = FALSE;
				// 	for (int i = 0; i < users_alive; i++)
				// 		memcpy(&clients[i].packet.apple, &user_packet.apple, sizeof(point));
				// }
				// else {
				// 	// pthread_t pid;
				// 	// pthread_create(&pid, NULL, apple_cpy, &clients[i].packet.apple);
				// 	memcpy(&user_packet.apple, &clients[i].packet.apple, sizeof(point));
				// 	for (int j = 0; j < users_alive; j++)
				// 		memcpy(&clients[j].packet.apple, &user_packet.apple, sizeof(point));
				// }
				memcpy(&user_packet.apple, &clients[i].packet.apple, sizeof(point));
				for (int j = 0; j < users_alive; j++)
					memcpy(&clients[j].packet.apple, &user_packet.apple, sizeof(point));
				clients[i].packet.generated_apple = 0;
				break;
			}
		}
		usleep(5000);
	}
}

void print_tail(point* tail, int count_tail, char symbol) {
	for (int i = 0; i < count_tail - 1; i++)
		mvaddch(tail[i].y, tail[i].x, symbol);
	mvaddch(tail[count_tail - 1].y, tail[count_tail - 1].x, symbol - 32);
}

void clear_tail(point* tail, int count_tail){
	for (int i = 0; i < count_tail; i++)
		mvaddch(tail[i].y, tail[i].x, ' ');
}

void new_tail(point* tail, int count_tail, int pos_x, int pos_y){
    for (int i = 0; i < count_tail - 1; i++)
        tail[i] = tail[i + 1];

	tail[count_tail - 1].x = pos_x;
	tail[count_tail - 1].y = pos_y;
}

int check_stop(int sx, int sy, point* first_tail, int first_tail_len, point* second_tail, int second_tail_len) {
	if (sx == 0 || sx == x || sy == 0 || sy == y)
		return 1;

	for (int i = 0; i < first_tail_len - 1; i++){
		if (sx == first_tail[i].x && sy == first_tail[i].y)
			return 1;
	}

    for (int i = 0; i < second_tail_len; i++){
		if (sx == second_tail[i].x && sy == second_tail[i].y)
			return 1;
	}

	return 0;
}

void start() {
	mvaddch(y / 2, x / 2, '3');
	refresh();
	usleep(1000000);
	mvaddch(y / 2, x / 2, '2');
	refresh();
	usleep(1000000);
	mvaddch(y / 2, x / 2, '1');
	refresh();
	usleep(1000000);
	mvaddch(y / 2, x / 2, ' ');
	refresh();
}
// змея увеличивается в тех же точках
// но на сервере яблоко отрисовывается, а на клиентах нет
// плюс на сервере змея не удлиняется
void print_and_clear_items(point* server_tail, int server_len, char symbol, point apple) {
	print_tail(server_tail, server_len, 's');
	for (int i = 0; i < users; i++)
		print_tail(clients[i].tail, clients[i].tail_len, symbol);
	if (show_new_apple)
		mvaddch(apple.y, apple.x, 'a');
	refresh();
	clear_tail(server_tail, server_len);
	for (int i = 0; i < users; i++)
		clear_tail(clients[i].tail, clients[i].tail_len);
	mvaddch(apple.y, apple.x, ' ');
}

void pre_game(point* server_tail, int len, char symbol) {
	print_tail(server_tail, len, 's');
	for (int i = 0; i < users; i++)
		print_tail(clients[i].tail, len, symbol);
	refresh();
	start();
}

point switch_direction(int sx, int sy, int current_direction) {
	switch (current_direction) {
		case 'w':
		case KEY_UP:
			sy -= 1;
			sy = sy >= 0 ? sy % y : sy + y;
			break;
		case KEY_DOWN:
		case 's':
			sy += 1;
			sy %= y;
			break;
		case KEY_LEFT:
		case 'a':
			sx -= 1;
			sx = sx >= 0 ? sx % x : sx + x;
			break;
		case KEY_RIGHT:
		case 'd':
			sx += 1;
			sx %= x;
			break;
		default:
			break;
	}

	point result;
	result.x = sx, result.y = sy;
	return result;
}

point gen_apple() {
	point new_apple;

	new_apple.x = rand() % (x - 2) + 1;
	new_apple.y = rand() % (y - 2) + 1;

	return new_apple;
}

void print_and_clear_apple(point apple) {
	mvaddch(apple.y, apple.x, 'a');
	refresh();
	mvaddch(apple.y, apple.x, ' ');
}

void realloc_tail(point* tail, int* tail_len, int pos_x, int pos_y) {
	// tail = realloc(tail, ((*tail_len) + 1) * sizeof(point));
	tail[(*tail_len)].x = pos_x;
	tail[(*tail_len)].y = pos_y;
	(*tail_len)++;
}

void game_server(int port, char symbol) {
	user_packet.direction = 'd';
	int sd;
	struct sockaddr_in addr;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0){
		perror("error calling socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		perror("bind");
		exit(1);
	}

	if (listen(sd, users)) {
		perror("listen");
		exit(1);
	}

	int count = 0;
	while(count < users) {
		clients[count].sockfd = accept(sd, NULL, NULL);
		clients[count].client_code = count + 1;
		count++;
	}

	for (int i = 0; i < users; i++)
		send(clients[i].sockfd, &clients[i].client_code, sizeof(uint8_t), 0);
	user_packet.user_code = 0;

	point* server_tail = malloc(LEN * sizeof(point));
	int tail_len = LEN;

	srand(time(0));
	int sx = 0, sy = 0;

	point* tmp_tail = malloc(LEN*sizeof(point));

	int* directions = malloc(users * sizeof(int));
	for (int i = 0; i < users; i++){
		if (rand() % 2 == 1) {
			sx = rand() % (x / 2) + 35, sy = rand() % (y - 2) + 1;
			for (int i = 0; i < LEN; i++)
				tmp_tail[LEN - i - 1].y = sy, tmp_tail[LEN - i - 1].x = sx + i;
			directions[i] = 'a';
		}
		else {
			sx = rand() % (x / 2) + 5, sy = rand() % (y - 2) + 1;
			for (int i = 0; i < LEN; i++)
				tmp_tail[LEN - i - 1].y = sy, tmp_tail[LEN - i - 1].x = sx - i;
			directions[i] = 'd';
		}

		clients[i].tail = malloc(LEN * 3 * sizeof(point));
		memcpy(clients[i].tail, tmp_tail, 10);
		clients[i].tail_len = LEN;

		clients[i].position.x = clients[i].tail[LEN - 1].x;
		clients[i].position.y = clients[i].tail[LEN - 1].y;
	}

	sx = rand() % (x / 2) + 5, sy = rand() % (y - 2) + 1;
	for (int i = 0; i < LEN; i++)
		tmp_tail[LEN - i - 1].y = sy, tmp_tail[LEN - i - 1].x = sx - i;

	memcpy(server_tail, tmp_tail, 10);
	free(tmp_tail);

	user_packet.apple = gen_apple();
	for (int i = 0; i < users; i++) {
		tail_info tmp;
		tmp.user_code = 0;
		tmp.direction = 'd';
		memcpy(&tmp.tail, server_tail, sizeof(point) * LEN);
		send(clients[i].sockfd, &tmp, sizeof(tail_info), 0);
		for (int j = 0; j < users; j++) {
			tmp.user_code = clients[j].client_code;
			tmp.direction = directions[j];
			memcpy(&tmp.tail, clients[j].tail, sizeof(point) * LEN);
			send(clients[i].sockfd, &tmp, sizeof(tail_info), 0);
		}
		send(clients[i].sockfd, &user_packet.apple, sizeof(point), 0);
		memcpy(&clients[i].packet.apple, &user_packet.apple, sizeof(point));
	}

	pthread_t pid;
	bool is_server = TRUE;
	pthread_create(&pid, NULL, get_and_send_packets, &is_server);

	pre_game(server_tail, LEN, symbol);

	// bool win = TRUE, new_apple = FALSE, done = FALSE;
	bool win = TRUE, done = FALSE;
	point switched_xy;
	while(TRUE){
		print_and_clear_items(server_tail, tail_len, symbol, user_packet.apple);

		for (int i = 0; i < users_alive; i++) {
			if (check_stop(server_tail[tail_len - 1].x, server_tail[tail_len - 1].y, server_tail, tail_len, clients[i].tail, clients[i].tail_len)){
				win = FALSE;
				user_packet.exit_code = 1;
				done = TRUE;
				break;
			}
		}
		for (int i = 0; i < users_alive; i++) {
			if (check_stop(clients[i].tail[clients[i].tail_len - 1].x, clients[i].tail[clients[i].tail_len - 1].y, clients[i].tail, clients[i].tail_len, server_tail, tail_len)){
				if (users_alive == 1) {
					done = TRUE;
					break;
				}
				else if (users_alive == 2) {
					if (clients[i].client_code == 1) {
						memcpy(&clients[i], &clients[i + 1], sizeof(client));
						// free(&clients[i + 1]);
						users_alive = 1;
					}
					else {
						// free(&clients[i]);
						users_alive = 1;
					}
				}
				else if (users_alive == 3) {
					if (clients[i].client_code == 1) {
						memcpy(&clients[i], &clients[i + 1], sizeof(client));
						memcpy(&clients[i + 1], &clients[i + 2], sizeof(client));
						// free(&clients[i + 2]);
						users_alive = 2;
					}
					else if (clients[i].client_code == 2) {
						memcpy(&clients[i + 1], &clients[i + 2], sizeof(client));
						// free(&clients[i + 2]);
						users_alive = 2;
					}
					else {
						// free(&clients[i + 2]);
						users_alive = 2;
					}
				}
			}
		}

		switched_xy = switch_direction(sx, sy, user_packet.direction);
		sx = switched_xy.x, sy = switched_xy.y;
		// for (int i = 0; i < users_alive; i++) {
		// 	switched_xy = switch_direction(clients[i].position.x, clients[i].position.y, clients[i].packet.direction);
		// 	clients[i].position.x = switched_xy.x, clients[i].position.y = switched_xy.y;
		// }

		if (sx == user_packet.apple.x && sy == user_packet.apple.y) {
			mvaddch(user_packet.apple.y, user_packet.apple.y, ' ');
			realloc_tail(server_tail, &tail_len, sx, sy);
			// pthread_t tmp_pid;
			// show_new_apple = FALSE;
			// bool q = FALSE;
			// pthread_create(&tmp_pid, NULL, wait_for_new_apple, &q);
			// new_apple = TRUE;
			user_packet.apple = gen_apple();
			user_packet.generated_apple = 1;
		}
		else
			new_tail(server_tail, tail_len, sx, sy);

		// if (new_apple) {
		// 	user_packet.apple = gen_apple();
		// 	user_packet.generated_apple = 1;
		// 	new_apple = FALSE;
		// 	// sent_apple = TRUE;
		// }

		for (int i = 0; i < users_alive; i++) {
			break;
			if (clients[i].position.x == user_packet.apple.x && clients[i].position.y == user_packet.apple.y) {
				realloc_tail(clients[i].tail, &clients[i].tail_len, clients[i].position.x, clients[i].position.y);
			}
			else
				new_tail(clients[i].tail, clients[i].tail_len, clients[i].position.x, clients[i].position.y);
		}

		if (done) break;
		usleep(80000);
	}

	clear();
	if (win)
		mvprintw(0, 0, "You win!");
	else {
		mvprintw(0, 0, "You lose!");
		mvprintw(1, 0, "Waiting for other players ...");
	}
	refresh();
	pthread_join(pid, NULL);
	usleep(2000000);

	close(sd);
	close(cd);
	endwin();
}

void game_client(int port, char* ip, char symbol) {
	user_packet.direction = 'a';
	struct sockaddr_in addr;

	cd = socket(AF_INET, SOCK_STREAM, 0);
	if (cd < 0){
		perror("error calling socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	if (connect(cd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		exit(1);
	}

	recv(cd, &user_packet.user_code, sizeof(uint8_t), 0);

	point* client_tail = malloc(LEN * sizeof(point));
	int tail_len = LEN;
	for (int i = 0; i < users; i++) {
		clients[i].tail = malloc(LEN * 3 * sizeof(point));
		clients[i].tail_len = LEN;
	}

	srand(time(0));

	tail_info* users_tails = malloc((users + 1) * sizeof(tail_info));
	for (int i = 0; i < users + 1; i++)
		recv(cd, &users_tails[i], sizeof(tail_info), 0);

	int count = 0;
	for (int i = 0; i < users + 1; i++) {
		if (users_tails[i].user_code == user_packet.user_code) {
			memcpy(client_tail, users_tails[i].tail, sizeof(point) * LEN);
			user_packet.direction = users_tails[i].direction;
		}
		else {
			memcpy(clients[count].tail, users_tails[i].tail, sizeof(point) * LEN);
			clients[count].client_code = users_tails[i].user_code;
			clients[i].packet.direction = users_tails[i].direction;
			count++;
		}
	}
	free(users_tails);

	recv(cd, &user_packet.apple, sizeof(point), 0);
	for (int i = 0; i < users; i++)
		memcpy(&clients[i].packet.apple, &user_packet.apple, sizeof(point));
	int sx = client_tail[LEN - 1].x;
	int sy = client_tail[LEN - 1].y;

	for (int i = 0; i < users; i++) {
		clients[i].position.x = clients[i].tail[LEN - 1].x;
		clients[i].position.y = clients[i].tail[LEN - 1].y;
	}

	pthread_t pid;
	bool is_server = FALSE;
	pthread_create(&pid, NULL, get_and_send_packets, &is_server);
	
	pre_game(client_tail, LEN, symbol);

	// bool win = TRUE, new_apple = FALSE, done = FALSE;
	bool win = TRUE, done = FALSE;
	point switched_xy;
	while(TRUE){
		print_and_clear_items(client_tail, tail_len, symbol, user_packet.apple);

		for (int i = 0; i < users_alive; i++) {
			if (check_stop(client_tail[tail_len - 1].x, client_tail[tail_len - 1].y, client_tail, tail_len, clients[i].tail, clients[i].tail_len)){
				win = FALSE;
				user_packet.exit_code = 1;
				done = TRUE;
				break;
			}
		}
		for (int i = 0; i < users_alive; i++) {
			if (check_stop(clients[i].tail[clients[i].tail_len - 1].x, clients[i].tail[clients[i].tail_len - 1].y, clients[i].tail, clients[i].tail_len, client_tail, tail_len) || (clients[i].packet.exit_code == 1)){
				if (users_alive == 1) {
					done = TRUE;
					break;
				}
				else if (users_alive == 2) {
					if (clients[i].client_code == 1) {
						memcpy(&clients[i], &clients[i + 1], sizeof(client));
						// free(&clients[i + 1]);
						users_alive = 1;
					}
					else {
						// free(&clients[i]);
						users_alive = 1;
					}
				}
				else if (users_alive == 3) {
					if (clients[i].client_code == 1) {
						memcpy(&clients[i], &clients[i + 1], sizeof(client));
						memcpy(&clients[i + 1], &clients[i + 2], sizeof(client));
						// free(&clients[i + 2]);
						users_alive = 2;
					}
					else if (clients[i].client_code == 2) {
						memcpy(&clients[i + 1], &clients[i + 2], sizeof(client));
						// free(&clients[i + 2]);
						users_alive = 2;
					}
					else {
						// free(&clients[i + 2]);
						users_alive = 2;
					}
				}
			}
		}

		// switched_xy = switch_direction(sx, sy, user_packet.direction);
		// sx = switched_xy.x, sy = switched_xy.y;
		for (int i = 0; i < users_alive; i++) {
			if (clients[i].client_code != 0) continue;
			switched_xy = switch_direction(clients[i].position.x, clients[i].position.y, clients[i].packet.direction);
			clients[i].position.x = switched_xy.x, clients[i].position.y = switched_xy.y;
		}

		// if (sx == user_packet.apple.x && sy == user_packet.apple.y) {
		// 	realloc_tail(client_tail, &tail_len, sx, sy);
		// 	new_apple = TRUE;
		// }
		// else
		// 	new_tail(client_tail, tail_len, sx, sy);

		// if (new_apple) {
		// 	user_packet.apple = gen_apple();
		// 	new_apple = FALSE;
		// 	sent_apple = TRUE;
		// }

		for (int i = 0; i < users_alive; i++) {
			if (clients[i].client_code != 0) continue;
			if (clients[i].position.x == user_packet.apple.x && clients[i].position.y == user_packet.apple.y) {
				mvaddch(user_packet.apple.y, user_packet.apple.y, ' ');
				realloc_tail(clients[i].tail, &clients[i].tail_len, clients[i].position.x, clients[i].position.y);
				// pthread_t tmp_pid;
				// show_new_apple = FALSE;
				// bool q = TRUE;
				// pthread_create(&tmp_pid, NULL, wait_for_new_apple, &q);
				// new_apple = TRUE;
			}
			else
				new_tail(clients[i].tail, clients[i].tail_len, clients[i].position.x, clients[i].position.y);
		}

		if (done) break;
		usleep(80000);
	}

	clear();
	if (win)
		mvprintw(0, 0, "You win!");
	else
		mvprintw(0, 0, "You lose!");
	refresh();
	usleep(2000000);

	close(cd);
	endwin();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n./game -p <port> -u <number of players> (for server)\n");
		printf("./game -p <port> -a <ip> -u <number of players> (for client)\n");
		printf("Number of players should not include server\n");
        exit(0);
    }

    int port;
	char* ip;
    int arg;
	bool server = TRUE;
    while ((arg = getopt(argc, argv, "p:a:u:")) != EOF) {
        switch (arg) {
        case 'p':
            port = atoi(optarg);
            break;
		case 'u':
            users = atoi(optarg);
            break;
        case 'a':
			server = FALSE;
            ip = optarg;
            break;
        }
    }

	if (users < 1 || users > 3) {
		printf("-u: should be in range 1-3\n");
		exit(0);
	}
	users_alive = users;

	initscr();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);

	pthread_t pid;
	pthread_create(&pid, NULL, direct, NULL);
	user_packet.exit_code = 0;

	for (int i = 0; i < y + 1; i++) mvaddch(i, 0, '|');
	for (int i = 0; i < x + 1; i++) mvaddch(0, i, '_');
	for (int i = 1; i < y + 1; i++) mvaddch(i, x, '|');
	for (int i = 1; i < x; i++) mvaddch(y, i, '_');

	clients = malloc(users * sizeof(client));
	for (int i = 0; i < users; i++)
		clients[i].client_code = i + 1;

	if (server) {
		user_packet.user_code = 0;
		game_server(port, 'c');
	}
	else {
		game_client(port, ip, 'c');
	}

	return 0;
}