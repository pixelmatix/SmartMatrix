#ifndef _PANEL_MAPS_H_
#define _PANEL_MAPS_H_

typedef struct PanelMappingEntry {
    int         rowOffset;
    int         bufferOffset;
    int         numPixels;
} PanelMappingEntry;

const PanelMappingEntry * getMultiRowRefreshPanelMap(unsigned char panelType);

#endif
