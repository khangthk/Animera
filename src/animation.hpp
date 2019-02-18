//
//  animation.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef animation_hpp
#define animation_hpp

#include <vector>
#include "cell.hpp"

using LayerIdx = uint32_t;
using FrameIdx = uint32_t;

struct CellPos {
  LayerIdx l;
  FrameIdx f;
};

struct CellRect {
  LayerIdx minL;
  LayerIdx maxL;
  FrameIdx minF;
  FrameIdx maxF;
};

using Frames = std::vector<CellPtr>;
using Layers = std::vector<Frames>;
using Frame = std::vector<Cell *>;

class Animation {
public:
  Animation(QSize, Format);
  explicit Animation(QIODevice *);
  
  void serialize(QIODevice *) const;

  bool hasLayer(LayerIdx) const;
  bool hasFrame(CellPos) const;
  bool hasFrame(LayerIdx, FrameIdx) const;
  LayerIdx layerCount() const;
  FrameIdx frameCount(LayerIdx) const;

  Cell *getCell(CellPos) const;
  Cell *getCell(LayerIdx, FrameIdx) const;
  Frame getFrame(FrameIdx) const;

  Layers copyRect(CellRect) const;
  void removeRect(CellRect);
  void clearRect(CellRect);
  void pasteRect(CellRect, const Layers &);

  void appendLayer();
  void appendSource(LayerIdx);
  void appendDuplicate(LayerIdx);
  void appendTransform(LayerIdx);

private:
  Layers layers;
  QSize size;
  Palette palette;
  Format format;
  
  const Cell *getLastCell(LayerIdx) const;
  void updateLayerInputs(LayerIdx);
  void removeTrailingNull(LayerIdx);
  void updateLayer(LayerIdx);
  bool validRect(CellRect) const;
};

#endif
