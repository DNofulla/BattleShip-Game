/*
	Daniel Nofulla
	Battleship Multiplayer Game
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define row 10
#define column 10

void error(char *c);
// struct for state hit or miss.
/*
	This structure records each movement.
	variable x and y indicate fired at.
	hitOrMiss indicates ship type
	Next points next structure.
*/
typedef struct node
{
	int x;
	int y;
	struct node *Next; // coordinates* node
	char hitOrMiss[5];

} coordinates;

static char userLetter; // user input letter
static int userDigit;	// user input digit
static int numberOfHit = 18;
static int countOfHit = 0;
static int EXIT = 1;
static int sockfd;
static int sockfd_inserver;
static int sockfd_1;
static int PORT = 8080;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

/*
 this function return sockfd
 */
void clientMode(char *addrs)
{
	printf("PORT: %d and IP: %s\n", PORT, addrs);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		error("Socket creation error from client!\n");
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(addrs);
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
	{
		error("Connection fail..!\n");
	}
	else
	{
		printf("Connection Success..!\n");
	}
}

void serverMode()
{
	sockfd_1 = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((bind(sockfd_1, (struct sockaddr *)&server_addr, sizeof(server_addr))) != 0)
	{
		error("Bind fail..\n");
	}
	else
	{
		printf("Bind Success..\n");
	}

	if (listen(sockfd_1, 5) != 0)
	{
		error("Listening fail.. \n");
	}
	else
	{
		printf("Listening... \n");
	}
	int length = sizeof(server_addr);
	sockfd_inserver = accept(sockfd_1, (struct sockaddr *)&client_addr, &length);
}

void receivedCoordinates(coordinates **head, coordinates **current, int x, int y, int argc)
{
	x = x - 65;
	// when temp's next is null, allocate new data at the next node and set the x and y.
	// if head is null, create new data
	int sendNum;
	int recevNum;
	char sendMess[4];
	char msg[2];
	char receive[2];
	bzero(receive, sizeof(receive));

	int sendValue = x * 10;
	sendValue += y;

	sprintf(msg, "%d", sendValue);
	if (sendValue < 10)
	{
		msg[1] = msg[0];
		msg[0] = 48;
	}

	if (argc == 3) // client version
	{

		sendNum = send(sockfd, msg, sizeof(msg), 0);
		if (sendNum == -1)
		{
			error("client: send fail\n");
		}

		recevNum = recv(sockfd, receive, sizeof(receive), 0);

		if (recevNum == -1)
		{
			error("client received message fail from server..\n");
		}
	}
	else if (argc == 2) // server version
	{
		recevNum = recv(sockfd_inserver, receive, sizeof(receive), 0);
		sendNum = send(sockfd_inserver, msg, sizeof(msg), 0);
		if (sendNum == -1)
			error("server: send fail\n");
	}

	if (NULL != *head)
	{
		(*current)->Next = (coordinates *)malloc(sizeof(coordinates));
		*current = (*current)->Next;
	}
	else
	{
		*head = (coordinates *)malloc(sizeof(coordinates));
		*current = *head;
	}

	int intReceive;
	sscanf(receive, "%d", &intReceive);

	int rec_x = intReceive / 10;
	int rec_y = intReceive % 10;
	printf("User2: %c%d!\n", rec_x + 65, rec_y);

	(*current)->x = rec_x;
	(*current)->y = rec_y;
	(*current)->Next = NULL;
}

void initialization(char **gameboard, int argc, char *addrs)
{

	int i, j, hx;

	int ship_number = 5;
	int ship3 = 0;

	srand(time(NULL));

	while (ship_number > 0)
	{
		i = rand() % 10; // prevent to escapte the gameboard
		j = rand() % 10; // when size of ship get smaller, more free to setted.

		if (gameboard[i][j] == '1' || gameboard[i] == '1' || &gameboard[j] == '1' || (i + ship_number) > 9 || (j + ship_number) > 9)
		{
			continue;
		}
		hx = rand() % 2; // horizontal or vertical

		if (hx == 0) // horizontal
		{
			int check;
			for (check = i; check < i + ship_number; check++)
			{
				if (gameboard[check][j] == '1')
				{
					check = -1;
					ship_number++;
					break;
				}
			}
			if (check != -1)
			{
				for (int k = 0; k < ship_number; k++)
				{
					gameboard[i + k][j] = '1';
				}
			}
		}
		if (hx == 1) // vertical
		{
			int check;
			for (check = j; check < j + ship_number; check++)
			{
				if (gameboard[i][check] == '1')
				{
					check = -1;
					ship_number++;
					break;
				}
			}
			if (check != -1)
			{
				for (int k = 0; k < ship_number; k++)
				{
					gameboard[i][j + k] = '1';
				}
			}
		}
		if (ship_number == 3)
		{
			ship3++;
		}

		if (ship3 != 1)
		{
			ship_number--;
		}
	}
	for (int i = 0; i < column; i++)
	{
		for (int j = 0; j < row; j++)
		{
			if (gameboard[i][j] != '1')
			{
				gameboard[i][j] = '0';
			}
		}
	}

	if (argc == 3)
	{
		clientMode(addrs);
	}
	else
	{
		serverMode();
	}
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}
/*
 This method make the game board free.
 */
void Teardown(char **gameboard, coordinates *head, int argc)
{
	FILE *logFile;
	printf("head's data %d\n", head->y);
	coordinates *temp;
	temp = head;
	printf("Here is your logged file\n");

	while (temp != NULL)
	{
		printf("Is it works?\n");

		logFile = fopen("logfile.log", "a+");
		if (logFile == NULL)
		{
			perror("Fail to open file\n");
		}
		else
		{
			printf("Fired at %c, %d. %s\n", temp->x + 65, temp->y, temp->hitOrMiss);
			fprintf(logFile, "Fired at %c, %d, %s\n", temp->x + 65, temp->y, temp->hitOrMiss);
		}
		temp = temp->Next;
	}
	close(sockfd);
	close(sockfd_1);
	close(sockfd_inserver);
}

/*accept a letter from A-J/ number from 0-9*/
void acceptInput(char **gameboard, coordinates **head, coordinates **current, int argc)
{
	printf("If you want to play BATTLESHIP, please Enter!\n");
	while (getchar() != '\n')
		;
	do
	{
		printf("Please enter a letter A-J: ");
		scanf("%c", &userLetter);
		if (userLetter < 65 || userLetter > 74 || (userLetter >= 97 && userLetter <= 106))
		{
			if (userLetter >= 97 && userLetter <= 106)
			{
				printf("You can only enter capital letter.\n");
			}
			else
			{
				printf("userLetter %d", userLetter);
				printf("\nInvalied letter.\n");
			}
			while ((getchar()) != '\n')
				;
		}

	} while (userLetter < 65 || userLetter > 74);

	int isDigit;
	do
	{
		printf("Please enter a number 0-9: ");
		isDigit = scanf("%d", &userDigit);
		// check error
		if (isDigit == 0) // check user typed number
		{
			printf("Invalied input. You can only enter a number.\n");
			while ((getchar()) != '\n')
				;
		}

		if (userDigit < 0 || userDigit > 9) // check user typed valid range of number
		{
			printf("Invalied number!\n");
			while ((getchar()) != '\n')
				;
		}
	} while (userDigit < 0 || userDigit > 9 || isDigit == 0);

	printf("----------------------BATTLE SHIP----------------------\n");
	receivedCoordinates(head, current, userLetter, userDigit, argc);
	UpdateState(gameboard, current, argc); // pass the structure to UpdateState method.
}

/*
	alter------> actually check the value entered.
*/
void UpdateState(char **gameboard, coordinates **current, int argc)
{
	// is hit or miss
	int x = (*current)->x;
	int y = (*current)->y;
	if (gameboard[x][y] == '1') // board shot 'Hit'
	{
		strcpy((*current)->hitOrMiss, "Hit");
		gameboard[x][y] = 'H';
		countOfHit++;
	}
	else if (gameboard[x][y] == '0') // board shot 'Miss'
	{
		strcpy((*current)->hitOrMiss, "Miss");
		gameboard[x][y] = 'M';
	}
	else
	{
		printf("It doesn't exist....\n");
	}
	DisplayStateOfTheWorld(gameboard, current, argc); // pass the structure to DisplayStateOfTheWorld
}

/*
	alter-->	print the game board and print the result of each move.
*/

/*print the result calculated in the state of the world*/
void DisplayStateOfTheWorld(char **gameboard, coordinates **current, int argc)
{

	for (int i = 0; i < column; i++)
	{
		for (int j = 0; j < row; j++)
		{
			printf("[%c]", gameboard[i][j]);
		}
		printf("\n");
	}

	printf("User2 : %s!!!!!..\n", (*current)->hitOrMiss);
	char receive[100];
	if (argc == 3) // client version
	{
		int sendNum = send(sockfd, (*current)->hitOrMiss, sizeof((*current)->hitOrMiss), 0);

		if (sendNum == -1)
		{
			error("client: H/M send fail\n");
		}

		int recevNum = recv(sockfd, receive, sizeof(receive), 0);
		if (recevNum == -1)
		{
			error("H/M receive fail..\n");
		}
		printf("\nMY Update: [%c%d]   %s\n", userLetter, userDigit, receive);
	}
	else if (argc == 2) // server version
	{
		int recevNum = recv(sockfd_inserver, receive, sizeof(receive), 0);
		if (recevNum == -1)
		{
			error("H/M receive fail..\n");
		}
		printf("\nMY Update: [%c%d]   %s\n", userLetter, userDigit, receive);
		int sendNum = send(sockfd_inserver, (*current)->hitOrMiss, sizeof((*current)->hitOrMiss), 0);
		if (sendNum == -1)
		{
			error("server: H/M send fail\n");
		}
	}

	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	int i = 0, j = 0;
	// this statement check the game is over or not
	if (numberOfHit == countOfHit)
	{
		printf("You win!\n");
		EXIT = 0;
	}
}
/*
	argv[1] for address, argv[2] for port,
	For example, ./client 197.0.0.1 8080
			 ./server 8080
*/
int main(int argc, char **argv)
{
	int port;
	if (argc == 3)
	{
		sscanf(argv[2], "%d", &port);
		printf("You are using client model.. \n");
	}
	else if (argc == 2)
	{
		sscanf(argv[1], "%d", &port);
		printf("You are using server model..\n");
	}
	else
		error("Command error: less argv\n");

	PORT = port;
	char **gameboard = (char **)malloc(column * row * sizeof(char *)); // allocation data of gameboard ad dimentio
	for (int i = 0; i < row; i++)
	{
		gameboard[i] = (char *)malloc(row * sizeof(char));
	}

	coordinates *head = NULL;
	coordinates *current = NULL;

	initialization(gameboard, argc, argv[1]); // initial

	while (EXIT)
	{
		acceptInput(gameboard, &head, &current, argc);
	}
	Teardown(gameboard, head, argc);
}

void error(char *c)
{
	perror(c);
	exit(1);
}
