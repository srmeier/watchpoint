/*
   stage00.c

   Copyright (C) 1997-1999, NINTENDO Co,Ltd.
*/

#include <assert.h>
#include <nusys.h>
#include <string.h>
#include <stdio.h>
#include "graphic.h"
#include "usb.h"
#include "commands.h"

void shadetri(Dynamic* dynamicp);
void shadetri_square(Square* square, int square_index);

/* Debug counters */
static u32 commands_received = 0;
static u32 usb_polls = 0;
static u32 usb_headers = 0;
static u32 last_datatype = 0;
static u32 last_size = 0;

/* PING/PONG messages */
#define MSG_PING 0x50494E47
#define MSG_PONG 0x504F4E47

/* Make the display list and activate the task. */

void makeDL00(void)
{
  u32 header;
  u8 datatype;
  u32 size;
  u8 command_buffer[512];
  Square* squares;
  int square_count;

  /* Check for USB commands */
  usb_polls++;
  header = usb_poll();
  if (header != 0)
  {
    usb_headers++;
    datatype = USBHEADER_GETTYPE(header);
    size = USBHEADER_GETSIZE(header);
    last_datatype = datatype;
    last_size = size;

    /* Handle binary commands for MCP */
    if (datatype == DATATYPE_RAWBINARY && size <= sizeof(command_buffer))
    {
      usb_read(command_buffer, size);

      /* Check for PING/PONG */
      if (size == 4) {
        u32 received_msg = (command_buffer[0] << 24) |
                          (command_buffer[1] << 16) |
                          (command_buffer[2] << 8) |
                          command_buffer[3];

        if (received_msg == MSG_PING) {
          /* Respond with PONG */
          u32 pong = MSG_PONG;
          usb_write(DATATYPE_RAWBINARY, &pong, sizeof(pong));
        }
      }
      /* Check for ECHO command */
      else if (size > 2 && command_buffer[0] == CMD_ECHO) {
        /* Format: [CMD:1][length:1][string:N] */
        u8 str_len = command_buffer[1];
        if (str_len > 0 && str_len <= size - 2) {
          /* Echo the string back as text */
          usb_write(DATATYPE_TEXT, &command_buffer[2], str_len);
          commands_received++;
        }
      }
      /* Check for ADD command (size can be 9 or 10 due to padding) */
      else if ((size == 9 || size == 10) && command_buffer[0] == CMD_ADD) {
        /* Format: [CMD:1][num1:4][num2:4] (may have 1 byte padding) */
        s32 num1 = (command_buffer[1] << 24) |
                   (command_buffer[2] << 16) |
                   (command_buffer[3] << 8) |
                   command_buffer[4];
        s32 num2 = (command_buffer[5] << 24) |
                   (command_buffer[6] << 16) |
                   (command_buffer[7] << 8) |
                   command_buffer[8];

        /* Add the numbers */
        s32 result = num1 + num2;

        /* Send result back as binary */
        usb_write(DATATYPE_RAWBINARY, &result, sizeof(result));
        commands_received++;
      }
      /* Check for ADD_SQUARE command (25 bytes, may be padded to 26) */
      else if ((size == 25 || size == 26) && command_buffer[0] == CMD_ADD_SQUARE) {
        commands_process(command_buffer, size);
        commands_received++;
      } else {
        /* Unknown command - increment a debug counter */
        commands_received++;  /* Count it anyway for debugging */
      }
    }
    /* Handle text commands (none currently, reserved for future use) */
    else if (datatype == DATATYPE_TEXT && size <= sizeof(command_buffer))
    {
      /* No text commands implemented yet */
      usb_skip(size);
    }
    else
    {
      /* Unknown data, just skip it */
      usb_skip(size);
    }
  }

  /* Get squares for display */
  squares = commands_get_squares();
  square_count = commands_get_square_count();

  /* Specify the display list buffer  */
  glistp = gfx_glist;

  /*  The initialization of RCP  */
  gfxRCPInit();

  /* Clear the frame buffer and the Z-buffer  */
  gfxClearCfb();

  /* The set of projection modeling matrices  */
  guOrtho(&gfx_dynamic.projection,
	  -(float)SCREEN_WD/2.0F, (float)SCREEN_WD/2.0F,
	  -(float)SCREEN_HT/2.0F, (float)SCREEN_HT/2.0F,
	  1.0F, 10.0F, 1.0F);
  guRotate(&gfx_dynamic.modeling, 0.0F, 0.0F, 0.0F, 1.0F);

  /* Draw all command squares */
  {
    int i;
    for (i = 0; i < MAX_SQUARES; i++) {
      if (squares[i].active) {
        shadetri_square(&squares[i], i);
      }
    }
  }

  /* End the construction of the display list  */
  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  /* Check if all are put in the array  */
  assert(glistp - gfx_glist < GFX_GLIST_LEN);

  /* Activate the RSP task.  Switch display buffers at the end of the task. */
  nuGfxTaskStart(gfx_glist,
		 (s32)(glistp - gfx_glist) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DEX , NU_SC_SWAPBUFFER);

  /* Display status on debug console */
  nuDebConTextPos(0, 2, 2);
  nuDebConCPuts(0, "N64 MCP Server");
  nuDebConTextPos(0, 2, 3);
  nuDebConPrintf(0, "Polls: %d Headers: %d", usb_polls, usb_headers);
  nuDebConTextPos(0, 2, 4);
  nuDebConPrintf(0, "Type: %02X Size: %d", last_datatype, last_size);
  nuDebConTextPos(0, 2, 5);
  nuDebConPrintf(0, "Cmds: %d Squares: %d", commands_received, square_count);
  nuDebConDisp(NU_SC_SWAPBUFFER);
}

/* The vertex coordinate  */
static Vtx shade_vtx[] =  {
        {        -64,  64, -5, 0, 0, 0, 0, 0xff, 0, 0xff	},
        {         64,  64, -5, 0, 0, 0, 0, 0, 0, 0xff	},
        {         64, -64, -5, 0, 0, 0, 0, 0, 0xff, 0xff	},
        {        -64, -64, -5, 0, 0, 0, 0xff, 0, 0, 0xff	},
};

/* Vertex array for command squares - 4 vertices per square, max 32 squares */
static Vtx square_vtx[MAX_SQUARES * 4];

/* Matrix storage for each square - need separate matrices for each transformation */
static Mtx square_modeling[MAX_SQUARES];
static Mtx square_rotate[MAX_SQUARES];
static Mtx square_translate[MAX_SQUARES];

/* Draw a square  */
void shadetri(Dynamic* dynamicp)
{
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->projection)),
		G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->modeling)),
		G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);

  gSPVertex(glistp++,&(shade_vtx[0]),4, 0);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++,G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++,G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++,G_SHADE| G_SHADING_SMOOTH);

  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
}

/* Draw a square from command data */
void shadetri_square(Square* square, int square_index)
{
  Vtx* vtx;
  s16 half_size;
  s16 x, y, z;

  /* Get pointer to this square's vertices in the global array */
  vtx = &square_vtx[square_index * 4];

  /* Get position and size */
  x = (s16)square->x;
  y = (s16)square->y;
  z = (s16)square->z;
  half_size = (s16)(square->size / 2);

  /* Create vertices centered at origin for proper rotation */
  vtx[0].v.ob[0] = -half_size;
  vtx[0].v.ob[1] = -half_size;
  vtx[0].v.ob[2] = 0;
  vtx[0].v.flag = 0;
  vtx[0].v.tc[0] = 0;
  vtx[0].v.tc[1] = 0;
  vtx[0].v.cn[0] = square->r;
  vtx[0].v.cn[1] = square->g;
  vtx[0].v.cn[2] = square->b;
  vtx[0].v.cn[3] = square->a;

  vtx[1].v.ob[0] = half_size;
  vtx[1].v.ob[1] = -half_size;
  vtx[1].v.ob[2] = 0;
  vtx[1].v.flag = 0;
  vtx[1].v.tc[0] = 0;
  vtx[1].v.tc[1] = 0;
  vtx[1].v.cn[0] = square->r;
  vtx[1].v.cn[1] = square->g;
  vtx[1].v.cn[2] = square->b;
  vtx[1].v.cn[3] = square->a;

  vtx[2].v.ob[0] = half_size;
  vtx[2].v.ob[1] = half_size;
  vtx[2].v.ob[2] = 0;
  vtx[2].v.flag = 0;
  vtx[2].v.tc[0] = 0;
  vtx[2].v.tc[1] = 0;
  vtx[2].v.cn[0] = square->r;
  vtx[2].v.cn[1] = square->g;
  vtx[2].v.cn[2] = square->b;
  vtx[2].v.cn[3] = square->a;

  vtx[3].v.ob[0] = -half_size;
  vtx[3].v.ob[1] = half_size;
  vtx[3].v.ob[2] = 0;
  vtx[3].v.flag = 0;
  vtx[3].v.tc[0] = 0;
  vtx[3].v.tc[1] = 0;
  vtx[3].v.cn[0] = square->r;
  vtx[3].v.cn[1] = square->g;
  vtx[3].v.cn[2] = square->b;
  vtx[3].v.cn[3] = square->a;

  /* Set projection matrix */
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&gfx_dynamic.projection),
            G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

  /* Build transformation matrix: rotate then translate */
  /* Use per-square matrix storage to avoid conflicts between squares */
  guRotate(&square_rotate[square_index], (float)square->rotation, 0.0F, 0.0F, 1.0F);
  guTranslate(&square_translate[square_index], (float)x, (float)y, (float)z);
  guMtxCatL(&square_rotate[square_index], &square_translate[square_index],
            &square_modeling[square_index]);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&square_modeling[square_index]),
            G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

  /* Load vertices */
  gSPVertex(glistp++, vtx, 4, 0);

  /* Set rendering mode */
  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
  gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);

  /* Draw two triangles to make a square */
  gSP2Triangles(glistp++, 0, 1, 2, 0, 0, 2, 3, 0);
}
