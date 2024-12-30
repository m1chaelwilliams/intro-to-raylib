#include <raylib.h>
#include <vector>

// clang-format off
int level[] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 1, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
};
// clang-format on

enum SpriteDirection {
  // -1 because when the sprite is facing left, multiplying the width by -1 will
  // mirror it horizontally
  Left = -1,
  Right = 1,
};

typedef struct Sprite {
  Texture2D texture;
  // x, y, width, and height of the sprite
  Rectangle dest_rect;
  Vector2 vel;
  SpriteDirection dir;
} Sprite;

void move_player(Sprite *player) {
  // reset the player's velocity to 0 every frame. this gives a snappy start and
  // stop effect. If you want gradual acceleration, multiply the velocity by a
  // float < 1.0f instead. You would also need to change the "=" to "+=/-=" when
  // the player moves.
  player->vel.x = 0.0;
  if (IsKeyDown(KEY_D)) {
    player->vel.x = 150.0;
    player->dir = SpriteDirection::Right;
  }
  if (IsKeyDown(KEY_A)) {
    player->vel.x = -150.0;
    player->dir = SpriteDirection::Left;
  }
  // note IsKeyPressed is different from IsKeyDown in that it is only
  // true when the key is pressed on the exact frame.
  if (IsKeyPressed(KEY_SPACE)) {
    player->vel.y = -300.0;
  }
}

// creates a smaller rectangle out of the destination rectangle.
// e.g. [px, py, 16.0, 24.0]
Rectangle player_hitbox(Sprite *player) {
  return (Rectangle){
      .x = player->dest_rect.x + 8.0f,
      .y = player->dest_rect.y + 8.0f,
      .width = 16.0f,
      .height = 24.0f,
  };
}

void apply_gravity(Sprite *sprite) {
  sprite->vel.y += 32.0;
  if (sprite->vel.y > 600.0) {
    sprite->vel.y = 600.0;
  }
}

void apply_vel_x(Sprite *sprite) {
  sprite->dest_rect.x += sprite->vel.x * GetFrameTime();
}

void apply_vel_y(Sprite *sprite) {
  sprite->dest_rect.y += sprite->vel.y * GetFrameTime();
}

void check_collisions_y(Sprite *sprite, std::vector<Sprite> &tiles) {

  Rectangle hitbox = player_hitbox(sprite);

  for (const auto &tile : tiles) {
    // if player's rect intersects tile's rect, do something!!!
    if (CheckCollisionRecs(hitbox, tile.dest_rect)) {
      // reverse the overlap

      // moving sprite is on bottom
      if (hitbox.y > tile.dest_rect.y) {
        sprite->dest_rect.y = tile.dest_rect.y + tile.dest_rect.height - 8.0f;
        // moving sprite is on the top
      } else {
        sprite->dest_rect.y = tile.dest_rect.y - sprite->dest_rect.height;
      }
    }
  }
}

void check_collisions_x(Sprite *sprite, std::vector<Sprite> &tiles) {
  Rectangle hitbox = player_hitbox(sprite);

  for (const auto &tile : tiles) {
    // if player's rect intersects tile's rect, do something!!!
    if (CheckCollisionRecs(hitbox, tile.dest_rect)) {
      // reverse the overlap

      // moving sprite is on the right
      if (hitbox.x > tile.dest_rect.x) {
        sprite->dest_rect.x = tile.dest_rect.x + tile.dest_rect.width - 8.0f;
        // moving sprite is on the left
      } else {
        sprite->dest_rect.x = tile.dest_rect.x - sprite->dest_rect.width + 8.0f;
      }
    }
  }
}

std::vector<Sprite> load_level(Texture2D temp_texture) {
  std::vector<Sprite> sprites;

  const int level_width = 8;
  const int level_height = 5;

  for (size_t i = 0; i < level_height * level_width; i++) {
    // converting an index to a coordinate
    // [_, _, _]
    // [_, _, _]
    // [_, o, _]
    // In this example, if index is at 'o', then its value is 7. the width of
    // the level is 3, so the x component of the coordinate would be 7 % 3 = 1.
    // The y component would be 7 / 3 = 2.
    size_t x = i % level_width;
    size_t y = i / level_width;

    // if number if > 0, then it is a sprite, add to a list
    if (level[i] > 0) {
      sprites.push_back((Sprite){.texture = temp_texture,
                                 .dest_rect = (Rectangle){.x = x * 32.0f,
                                                          .y = y * 32.0f,
                                                          .width = 32.0f,
                                                          .height = 32.0f}});
    }
  }

  return sprites;
}

int main() {

  // init app
  InitWindow(600, 400, "awesome window");
  SetTargetFPS(60);

  Texture2D player_idle_texture =
      LoadTexture("assets/heros/herochar_idle_anim_strip_4.png");
  Texture2D tiles_texture = LoadTexture("assets/tiles/tileset.png");

  Sprite player = (Sprite){.texture = player_idle_texture,
                           .dest_rect =
                               (Rectangle){
                                   .x = 10.0,
                                   .y = -200.0,
                                   .width = 32.0,
                                   .height = 32.0,
                               },
                           .dir = SpriteDirection::Right};

  std::vector<Sprite> level_tiles = load_level(tiles_texture);

  // run app
  while (!WindowShouldClose()) {

    // update section
    move_player(&player);
    apply_gravity(&player);

    // after all movement updates
    apply_vel_y(&player);
    check_collisions_y(&player, level_tiles);
    apply_vel_x(&player);
    check_collisions_x(&player, level_tiles);

    // hack to keep player from falling

    // if below ground, put back on ground
    if (player.dest_rect.y > GetScreenHeight() - player.dest_rect.height) {
      player.dest_rect.y = GetScreenHeight() - player.dest_rect.height;
    }

    // draw section
    BeginDrawing();

    // all drawing happens
    ClearBackground(SKYBLUE);

    // draw the level tiles
    for (const auto &tile : level_tiles) {
      DrawTexturePro(tile.texture, {0, 0, 16, 16}, tile.dest_rect, {0, 0}, 0.0,
                     RAYWHITE);
    }

    DrawTexturePro(player.texture,
                   {0, 0, 16 * static_cast<float>(player.dir), 16},
                   player.dest_rect, {0, 0}, 0.0, RAYWHITE);

    EndDrawing();
  }

  // free your memory!!!
  UnloadTexture(player_idle_texture);
  UnloadTexture(tiles_texture);

  // close app
  CloseWindow();

  return 0;
}
