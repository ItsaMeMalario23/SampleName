#ifndef RENDER_H
#define RENDER_H

#include <main.h>
#include <com.h>

void init(context_t* context);
void chmode(u32 mode);
void draw(context_t* context);
void cleanup(context_t* context);

#endif