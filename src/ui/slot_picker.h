#ifndef SLOT_PICKER_H
#define SLOT_PICKER_H

#include <SDL2/SDL.h>

typedef enum {
    SLOT_RESULT_NONE = 0,
    SLOT_RESULT_SELECTED,
    SLOT_RESULT_CANCEL
} SlotResult;

typedef struct {
    int is_open;
    int is_save;
} SlotPicker;

void slot_picker_open(SlotPicker *sp, int is_save);
SlotResult slot_picker_handle_event(SlotPicker *sp, const SDL_Event *e, int sw, int sh, int *out_slot);
void slot_picker_render(SDL_Renderer *r, const SlotPicker *sp, int sw, int sh);

#endif
