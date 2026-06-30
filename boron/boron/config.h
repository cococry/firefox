#pragma once

#include <leif/color.h>
#include <leif/ez_api.h>
#include <leif/layout.h>
#include <leif/ui_core.h>
#include <ragnar/api.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

extern void uidesktops(void);
extern void uicmds(void);

typedef enum {
  BarLeft = 0,
  BarRight,
  BarTop,
  BarBottom
} barmode_t;

typedef enum {
  BarPosStart = 0,
  BarPosCenter,
  BarPosEnd
} barpos_t;

typedef struct {
  char* cmd;
  float update_interval_secs;
} barcmd_t;

typedef enum {
  ewmh_atom_none = 0,
  ewmh_atom_desktop_names,
  ewmh_atom_current_desktop,
  ewmh_atom_count
} ewmh_atom_t;

typedef struct {
  int32_t x, y;
  uint32_t width, height;
} area_t;

typedef struct {
  Display* dpy;
  Window root;
  lf_ui_state_t* ui;

  area_t bararea;

  uint32_t crntdesktop;
  char** desktopnames;
  uint32_t numdesktops;

  Atom ewmh_atoms[ewmh_atom_count];

  lf_div_t* div_desktops;

  char** cmdoutputs;
} state_t;

static barcmd_t barcmds[] = {
  {
    .cmd = "boron-ram",
    .update_interval_secs = 5.0f
  },
  {
    .cmd = "boron-disk",
    .update_interval_secs = 20.0f
  },
  {
    .cmd = "boron-weather",
    .update_interval_secs = 300.0f
  },
  {
    .cmd = "boron-time",
    .update_interval_secs = 1.0f
  },
};

static state_t s;

/*
  Hard black/white palette:

  bg:      #000000
  fg:      #ffffff
  muted:   #ffffff
  line:    #ffffff
  accent:  #ffffff

  No colors.
  No softness.
  No rounded corners.
*/

static const char*      barfont             = "Iosevka Nerd Font";
static const int32_t    barmon              = -1;
static const barmode_t  barmode             = BarTop;

static const uint32_t   barmargin           = 0;
static const uint32_t   barsize             = 36;
static const uint32_t   barborderwidth      = 5;

/* Fully opaque bar */
static const uint32_t   barcolor_window     = 0x000000;
static const uint32_t   bar_alpha           = 255;

static const uint32_t   barcolor_primary    = 0x000000;
static const uint32_t   barcolor_secondary  = 0x111111;
static const uint32_t   barcolor_faint      = 0x000000;
static const uint32_t   barcolor_muted      = 0xFFFFFF;
static const uint32_t   barcolor_accent     = 0xFFFFFF;
static const uint32_t   bartextcolor        = 0xFFFFFF;

void bar_style_widget(lf_ui_state_t* ui, lf_widget_t* widget) {
  lf_style_widget_prop_color(
    ui,
    widget,
    color,
    lf_color_from_hex(barcolor_primary)
  );

  lf_style_widget_prop_color(
    ui,
    widget,
    border_color,
    lf_color_from_hex(barcolor_secondary)
  );

  lf_style_widget_prop(ui, widget, border_width, 0);
  lf_style_widget_prop(ui, widget, corner_radius_percent, 0);

  lf_widget_set_padding(ui, widget, 0);
  lf_style_widget_prop(ui, widget, padding_left, 6);
  lf_style_widget_prop(ui, widget, padding_right, 6);
}

void bar_layout(lf_ui_state_t* ui) {
  lf_div(ui);
  lf_widget_set_fixed_height_percent(ui, lf_crnt(ui), 100.0f);
  lf_widget_set_alignment(lf_crnt(ui), AlignCenterVertical);
  lf_widget_set_padding(ui, lf_crnt(ui), 0);
  lf_crnt(s.ui)->scrollable = false;

  lf_div(ui);
  lf_widget_set_layout(lf_crnt(ui), LayoutHorizontal);
  lf_widget_set_alignment(lf_crnt(ui), AlignCenterVertical);
  lf_component(ui, uidesktops);

  lf_div(ui);
  lf_widget_set_layout(lf_crnt(ui), LayoutHorizontal);
  lf_widget_set_sizing(lf_crnt(ui), SizingGrow);
  lf_div_end(ui);

  lf_component(ui, uicmds);

  lf_div_end(ui);
}

void set_prop(lf_widget_t* widget, float* prop, float val) {
  lf_widget_add_animation(
    widget,
    prop,
    *prop,
    val,
    0.12,
    lf_ease_out_quad
  );
}

void bar_desktop_hover(lf_ui_state_t* ui, lf_widget_t* widget) {
  lf_widget_set_prop(ui, widget, &widget->props.padding_left, 8);
  lf_widget_set_prop(ui, widget, &widget->props.padding_right, 8);

  lf_widget_set_fixed_width(ui, widget, 55);
  lf_widget_set_visible(widget->childs[0], true);

  lf_style_widget_prop_color(
    ui,
    widget,
    color,
    lf_color_from_hex(barcolor_primary)
  );

  lf_style_widget_prop_color(
    ui,
    widget,
    border_color,
    lf_color_from_hex(barcolor_accent)
  );

  lf_style_widget_prop(ui, widget, border_width, 1);
  lf_style_widget_prop(ui, widget, corner_radius_percent, 0);
}

void bar_desktop_leave(lf_ui_state_t* ui, lf_widget_t* widget) {
  lf_component_rerender(s.ui, uidesktops);
}

void bar_desktop_click(lf_ui_state_t* ui, lf_widget_t* widget) {
  rg_cmd_switch_desktop(*(int32_t*)widget->user_data);
}

void bar_desktop_design(
  lf_ui_state_t* ui,
  uint32_t desktop,
  uint32_t crntdesktop,
  const char* name
) {
  uint32_t dist = abs((int32_t)desktop - (int32_t)crntdesktop);
  bool active = desktop == crntdesktop;

  lf_button(ui);

  lf_widget_set_padding(ui, lf_crnt(ui), 0);
  lf_widget_set_transition_props(lf_crnt(ui), 0.12f, lf_ease_out_quad);

  lf_style_widget_prop_color(
    ui,
    lf_crnt(ui),
    color,
    lf_color_from_hex(active ? 0x999999 : 0x333333)
  );

  lf_style_widget_prop_color(
    ui,
    lf_crnt(ui),
    border_color,
    lf_color_from_hex(active ? 0x999999 : 0x000000)
  );

  lf_style_widget_prop(ui, lf_crnt(ui), border_width, active ? 1 : 0);
  lf_style_widget_prop(ui, lf_crnt(ui), corner_radius_percent, 0);

  lf_widget_set_fixed_width(
    ui,
    lf_crnt(ui),
    active ? 55 : MAX(18, 22 - (int32_t)(dist * 2))
  );

  lf_widget_set_fixed_height(ui, lf_crnt(ui), 22);

  ((lf_button_t*)lf_crnt(ui))->on_enter = bar_desktop_hover;
  ((lf_button_t*)lf_crnt(ui))->on_leave = bar_desktop_leave;
  ((lf_button_t*)lf_crnt(ui))->on_click = bar_desktop_click;

  uint32_t* data = malloc(sizeof(uint32_t));
  *data = desktop;
  lf_crnt(ui)->user_data = data;

  lf_text_h4(ui, name);

  lf_style_widget_prop_color(
    ui,
    lf_crnt(ui),
    text_color,
    lf_color_from_hex(active ? 0x000000 : 0xFFFFFF)
  );

  lf_crnt(ui)->visible = active;

  lf_button_end(ui);
}
