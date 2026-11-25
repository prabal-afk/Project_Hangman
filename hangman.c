/*
 * Hangman - Console Word Guessing Game
 * FOCP-I (C Programming) Mini Project
 *
 * The computer picks a secret word from a chosen category. The player guesses
 * one letter at a time and must find the whole word before making
 * MAX_WRONG incorrect guesses.
 *
 * Compile: gcc -Wall -Wextra hangman.c -o hangman
 * Run:     ./hangman        (Windows: hangman.exe)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_WRONG           6    /* incorrect guesses allowed            */
#define NUM_CATEGORIES      3
#define WORDS_PER_CATEGORY  8
#define INPUT_LEN           64   /* size of the input buffer             */
#define ALPHABET            26

/* ---------- Word database: 2D array of strings (category x word) ---------- */
static const char *category_names[NUM_CATEGORIES] = {
    "Animals", "Fruits and Vegetables", "Programming"
};

static const char *word_list[NUM_CATEGORIES][WORDS_PER_CATEGORY] = {
    { "elephant", "giraffe", "kangaroo", "penguin",
      "dolphin", "cheetah", "buffalo", "squirrel" },
    { "banana", "pineapple", "cucumber", "broccoli",
      "mango", "strawberry", "pumpkin", "spinach" },
    { "variable", "function", "pointer", "compiler",
      "array", "loop", "string", "keyboard" }
};

/* ---------- Hangman drawings: one picture per number of wrong guesses ----- */
static const char *hangman_stages[MAX_WRONG + 1] = {
    "  +---+\n  |   |\n      |\n      |\n      |\n      |\n=========",
    "  +---+\n  |   |\n  O   |\n      |\n      |\n      |\n=========",
    "  +---+\n  |   |\n  O   |\n  |   |\n      |\n      |\n=========",
    "  +---+\n  |   |\n  O   |\n /|   |\n      |\n      |\n=========",
    "  +---+\n  |   |\n  O   |\n /|\\  |\n      |\n      |\n=========",
    "  +---+\n  |   |\n  O   |\n /|\\  |\n /    |\n      |\n=========",
    "  +---+\n  |   |\n  O   |\n /|\\  |\n / \\  |\n      |\n========="
};

/* ---------- Function prototypes ---------- */
void show_menu(void);
void show_instructions(void);
void show_scores(int wins, int losses, int points);
int  read_line(char *buffer, int size);
int  get_menu_choice(int low, int high);
int  choose_category(void);
const char *pick_word(int category);
void draw_hangman(int wrong);
void print_word_state(const char *word, const int *guessed);
void print_guessed_letters(const int *guessed);
int  is_word_guessed(const char *word, const int *guessed);
int  reveal_hint(const char *word, int *guessed);
int  play_round(const char *word, int category, int *points);

/* =========================== main =========================== */
int main(void)
{
    int choice, category, result;
    int wins = 0, losses = 0, points = 0;
    const char *secret;

    srand((unsigned int)time(NULL));   /* seed the random number generator */

    do {
        show_menu();
        choice = get_menu_choice(1, 4);

        switch (choice) {
        case 1:
            category = choose_category();
            secret   = pick_word(category);
            result   = play_round(secret, category, &points);
            if (result == 1) {
                wins++;
            } else {
                losses++;
            }
            break;
        case 2:
            show_instructions();
            break;
        case 3:
            show_scores(wins, losses, points);
            break;
        case 4:
            printf("\nThanks for playing Hangman. Goodbye!\n");
            break;
        }
    } while (choice != 4);

    return 0;
}

/* ====================== Menus and messages ====================== */
void show_menu(void)
{
    printf("\n==============================\n");
    printf("         H A N G M A N\n");
    printf("==============================\n");
    printf("  1. Play Game\n");
    printf("  2. How to Play\n");
    printf("  3. View Scores\n");
    printf("  4. Exit\n");
    printf("------------------------------\n");
}

void show_instructions(void)
{
    printf("\n------------- HOW TO PLAY -------------\n");
    printf("1. Choose a category; the computer picks a secret word.\n");
    printf("2. Each hidden letter is shown as an underscore ( _ ).\n");
    printf("3. Type ONE letter per turn and press Enter.\n");
    printf("4. A correct letter is revealed in every position it appears.\n");
    printf("5. A wrong letter adds a part to the hangman. You can make\n");
    printf("   only %d wrong guesses before you lose.\n", MAX_WRONG);
    printf("6. Type '?' once per game for a hint (costs 5 points).\n");
    printf("7. Win = 10 points per remaining chance.\n");
    printf("---------------------------------------\n");
}

void show_scores(int wins, int losses, int points)
{
    printf("\n------------ SCOREBOARD ------------\n");
    printf("  Games won  : %d\n", wins);
    printf("  Games lost : %d\n", losses);
    printf("  Total points: %d\n", points);
    printf("------------------------------------\n");
}

/* ======================== Input handling ======================== */
/*
 * Reads one line safely using fgets, removes the newline, and discards any
 * extra characters if the user typed more than the buffer can hold.
 * Returns 1 on success, 0 on end-of-input (e.g. Ctrl+D / Ctrl+Z).
 */
int read_line(char *buffer, int size)
{
    int ch;
    size_t len;

    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }
    len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* throw away the rest of the long line */
        }
    }
    return 1;
}

/* Keeps asking until the user enters a whole number between low and high. */
int get_menu_choice(int low, int high)
{
    char line[INPUT_LEN];
    char *end;
    long value;

    while (1) {
        printf("Enter your choice (%d-%d): ", low, high);
        if (!read_line(line, INPUT_LEN)) {
            printf("\nInput closed. Exiting.\n");
            exit(0);
        }
        value = strtol(line, &end, 10);
        if (end != line && *end == '\0' && value >= low && value <= high) {
            return (int)value;
        }
        printf("Invalid input! Please enter a number from %d to %d.\n",
               low, high);
    }
}

int choose_category(void)
{
    int i;

    printf("\nChoose a category:\n");
    for (i = 0; i < NUM_CATEGORIES; i++) {
        printf("  %d. %s\n", i + 1, category_names[i]);
    }
    return get_menu_choice(1, NUM_CATEGORIES) - 1;
}

/* Returns a pointer to a random word of the chosen category. */
const char *pick_word(int category)
{
    return word_list[category][rand() % WORDS_PER_CATEGORY];
}

/* ========================= Game display ========================= */
void draw_hangman(int wrong)
{
    printf("\n%s\n", hangman_stages[wrong]);
}

/* Prints the word with underscores for letters not guessed yet. */
void print_word_state(const char *word, const int *guessed)
{
    const char *p;

    printf("\nWord: ");
    for (p = word; *p != '\0'; p++) {
        if (guessed[*p - 'a']) {
            printf("%c ", toupper((unsigned char)*p));
        } else {
            printf("_ ");
        }
    }
    printf("\n");
}

void print_guessed_letters(const int *guessed)
{
    int i, any = 0;

    printf("Guessed letters: ");
    for (i = 0; i < ALPHABET; i++) {
        if (guessed[i]) {
            printf("%c ", 'A' + i);
            any = 1;
        }
    }
    if (!any) {
        printf("(none yet)");
    }
    printf("\n");
}

/* ========================== Game logic ========================== */
/* Returns 1 if every letter of the word has been guessed. */
int is_word_guessed(const char *word, const int *guessed)
{
    const char *p;

    for (p = word; *p != '\0'; p++) {
        if (!guessed[*p - 'a']) {
            return 0;
        }
    }
    return 1;
}

/* Reveals the first hidden letter; returns that letter. */
int reveal_hint(const char *word, int *guessed)
{
    const char *p;

    for (p = word; *p != '\0'; p++) {
        if (!guessed[*p - 'a']) {
            guessed[*p - 'a'] = 1;
            return toupper((unsigned char)*p);
        }
    }
    return 0;
}

/*
 * Plays one complete round.
 * Returns 1 if the player won, 0 if the player lost.
 * 'points' is updated through a pointer.
 */
int play_round(const char *word, int category, int *points)
{
    int  guessed[ALPHABET] = {0};
    int  wrong = 0, hint_used = 0, i, found, hint_letter;
    char line[INPUT_LEN];
    char letter;
    int  length = (int)strlen(word);

    printf("\nCategory: %s  |  The word has %d letters.\n",
           category_names[category], length);

    while (wrong < MAX_WRONG && !is_word_guessed(word, guessed)) {
        draw_hangman(wrong);
        print_word_state(word, guessed);
        print_guessed_letters(guessed);
        printf("Chances left: %d\n", MAX_WRONG - wrong);
        printf("Guess a letter (or ? for a hint): ");

        if (!read_line(line, INPUT_LEN)) {
            printf("\nInput closed. Exiting.\n");
            exit(0);
        }

        /* ---- hint request ---- */
        if (strcmp(line, "?") == 0) {
            if (hint_used) {
                printf(">> You already used your hint for this game.\n");
            } else {
                hint_letter = reveal_hint(word, guessed);
                hint_used = 1;
                printf(">> Hint: the letter '%c' is in the word.\n",
                       hint_letter);
            }
            continue;
        }

        /* ---- input validation: exactly one alphabetic character ---- */
        if (strlen(line) != 1 || !isalpha((unsigned char)line[0])) {
            printf(">> Invalid input! Enter a single letter (A-Z).\n");
            continue;
        }

        letter = (char)tolower((unsigned char)line[0]);

        if (guessed[letter - 'a']) {
            printf(">> You already guessed '%c'. Try another letter.\n",
                   toupper((unsigned char)letter));
            continue;
        }

        guessed[letter - 'a'] = 1;

        /* ---- check whether the letter is in the word ---- */
        found = 0;
        for (i = 0; i < length; i++) {
            if (word[i] == letter) {
                found = 1;
            }
        }

        if (found) {
            printf(">> Good guess! '%c' is in the word.\n",
                   toupper((unsigned char)letter));
        } else {
            wrong++;
            printf(">> Sorry, '%c' is not in the word.\n",
                   toupper((unsigned char)letter));
        }
    }

    /* ---- result ---- */
    if (is_word_guessed(word, guessed)) {
        int earned = (MAX_WRONG - wrong) * 10 - (hint_used ? 5 : 0);
        if (earned < 0) {
            earned = 0;
        }
        *points += earned;
        print_word_state(word, guessed);
        printf("\n*** CONGRATULATIONS! You guessed the word! ***\n");
        printf("You earned %d points.\n", earned);
        return 1;
    }

    draw_hangman(wrong);
    printf("\n*** GAME OVER! You ran out of chances. ***\n");
    printf("The secret word was: ");
    for (i = 0; i < length; i++) {
        printf("%c", toupper((unsigned char)word[i]));
    }
    printf("\n");
    return 0;
}
