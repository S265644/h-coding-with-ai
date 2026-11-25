#include "singly_linked_list.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

constexpr size_t TRACK_TITLE_SIZE = 60;

#define PLAYLIST_IN_PATH "playlist.txt"
#define PLAYLIST_OUT_PATH "playlist-out.txt"
// To avoid unnecessary complexity, we fix the filenames instead of getting them
// through runtime parameters.

typedef char Track[TRACK_TITLE_SIZE];
Node *playlist;

int move_song(Node **list, size_t from, size_t to);
int find_song(Node *list, const char *title);
void free_playlist(Node **list);

/// Removes trailing newline from the line, if it exists.
/// Note: Some lines may not have a newline, e.g., last line in a file,
/// therefore we have to check for presence.
char *remove_newline_if_exists(char *line) {
  if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
    line[strlen(line) - 1] = '\0';

  return line;
}

/// Reads lines from at `filename`, creates a node for each line and inserts
/// nodes to `list`.
Node **load_file(const char *filename, Node **list) {
  auto f = fopen(filename, "r");

  if (!f) {
    perror(PLAYLIST_IN_PATH);
    exit(EXIT_FAILURE);
  }

  char line[TRACK_TITLE_SIZE];

  while (fgets(line, TRACK_TITLE_SIZE, f)) {
    remove_newline_if_exists(line);

    if (strlen(line) == 0)
      continue;

    auto new_node = (Node *)malloc(sizeof(Node));

    if (!new_node) {
      perror("Memory allocation failed");
      fclose(f);
      free_playlist(list);
      exit(EXIT_FAILURE);
    }

    new_node->next = nullptr;

    auto data = (Track *)malloc(sizeof(Track));

    if (!data) {
      perror("Memory allocation failed");
      free(new_node);
      fclose(f);
      free_playlist(list);
      exit(EXIT_FAILURE);
    }

    new_node->data = data;

    strcpy(new_node->data, line);
    insert_at(list, list_len(*list), new_node);
  }

  fclose(f);
  return list;
}

/// Saves `list` contents to the file at `filename`.
void save_file(const char *filename, Node *list) {
  auto f = fopen(filename, "w");

  if (!f) {
    perror(filename);
    return;
  }

  // Move through the list and save the tracks to the file
  // Note: You have to cast the data to print the track to the file as follows:
  // `*(Data *)current->data`, which is the same as `(char *)current->data`.
  // We need this cast, because `data` is a pointer to everything (`void *`).
  Node *current = list;

  while (current) {
    fprintf(f, "%s\n", *(Track *)current->data);
    current = current->next;
  }

  // or less efficient:
  /* for (size_t i = 0; i < list_len(playlist); ++i) {
    fprintf(f, "%s\n", (char *)current->data);
    current = current->next;
  } */

  fclose(f);
}

void print_tracks(const Node *const playlist) {
  auto current = playlist;

  for (size_t i = 1; current; i++, current = current->next)
    printf("%2zu: %s\n", i, (char *)current->data);
}

int main() {

#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  load_file(PLAYLIST_IN_PATH, &playlist);

  puts("Loaded tracks:");
  print_tracks(playlist);

  // Deletion
  size_t node_index_to_del = 4;

  if (list_len(playlist) > node_index_to_del) {
    free(delete_at(&playlist, node_index_to_del));
  }

  // Insertion
  Node *node = (Node *)malloc(sizeof(Node));

  if (!node) {
    free_playlist(&playlist);
    return EXIT_FAILURE;
  }

  node->data = malloc(sizeof(Track));
  node->next = nullptr;

  if (!node->data) {
    free(node);
    free_playlist(&playlist);
    return EXIT_FAILURE;
  }

  strcpy(node->data, "Tarkan – Şımarık 💋");

  if (list_len(playlist) >= 3) {
    insert_at(&playlist, 3, node);
  } else {
    insert_at(&playlist, list_len(playlist), node);
  }

  puts("\nPlaylist after deletion and insertion:");
  print_tracks(playlist);

  puts("\nMoving song up:");

  if (move_song(&playlist, 1, 0)) {
    print_tracks(playlist);
  } else {
    puts("Invalid song position.");
  }

  puts("\nMoving song down:");

  if (move_song(&playlist, 0, 1)) {
    print_tracks(playlist);
  } else {
    puts("Invalid song position.");
  }

  puts("\nSearching for a song:");

  int position = find_song(playlist, "Tarkan – Şımarık 💋");

  if (position != -1) {
    printf("Song found at position %d\n", position);
  } else {
    puts("Song not found.");
  }

  save_file(PLAYLIST_OUT_PATH, playlist);

  free_playlist(&playlist);

  return EXIT_SUCCESS;
}

int move_song(Node **list, size_t from, size_t to) {

  if (!list || !*list) {
    return 0;
  }

  size_t length = list_len(*list);

  if (from >= length || to >= length) {
    return 0;
  }

  if (from == to) {
    return 1;
  }

  Node **source = list;

  for (size_t i = 0; i < from; i++) {
    source = &(*source)->next;
  }

  Node *song = *source;
  *source = song->next;

  Node **destination = list;

  for (size_t i = 0; i < to; i++) {
    destination = &(*destination)->next;
  }

  song->next = *destination;
  *destination = song;

  return 1;
}

int find_song(Node *list, const char *title) {

  Node *current = list;
  int position = 1;

  while (current) {

    if (strcmp((char *)current->data, title) == 0) {
      return position;
    }

    current = current->next;
    position++;
  }

  return -1;
}

void free_playlist(Node **list) {

  while (*list) {

    Node *current = *list;
    *list = current->next;

    free(current->data);
    free(current);
  }
}