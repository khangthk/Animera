//
//  timeline cells widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_cells_widget_hpp
#define timeline_cells_widget_hpp

#include "cell.hpp"
#include <QtCore/qtimer.h>
#include <QtWidgets/qwidget.h>
#include "scroll bar widget.hpp"

class TimelineWidget;
class QVBoxLayout;

class LayerCellsWidget final : public QWidget {
  Q_OBJECT
  
public:
  // @TODO a sparse data structure might be better
  struct LinkedSpan {
    // Does this need to be a std::unique_ptr?
    // We need stable pointers to cells
    CellPtr cell;
    FrameIdx len = 1;
  };

  LayerCellsWidget(QWidget *, TimelineWidget *);
  
  void insertFrame(FrameIdx);
  void insertNullFrame(FrameIdx);
  void removeFrame(FrameIdx);
  void clearFrames(FrameIdx);
  void swapWith(LayerCellsWidget &);
  void cellFromNull(FrameIdx);
  
  void appendCell(FrameIdx = 1);
  void appendNull(FrameIdx = 1);
  void appendFrame();
  Cell *getCell(FrameIdx);
  
  void serialize(QIODevice *) const;
  void deserialize(QIODevice *);

private:
  TimelineWidget &timeline;
  std::vector<LinkedSpan> frames;
  QPixmap cellPix;
  QPixmap beginLinkPix;
  QPixmap endLinkPix;
  
  void loadIcons();
  void addSize(FrameIdx);
  const Cell *getLastCell() const;
  CellPtr makeCell() const;
  
  void paintBorder(QPainter &, int);
  void paintEvent(QPaintEvent *) override;
};

class CellsWidget final : public QWidget {
  Q_OBJECT
  
public:
  CellsWidget(QWidget *, TimelineWidget *);

Q_SIGNALS:
  void resized();
  void posChanged(Cell *, LayerIdx, FrameIdx);
  void frameChanged(const Frame &);
  void ensureVisible(QPoint);

public Q_SLOTS:
  void nextFrame();
  void prevFrame();
  void layerBelow();
  void layerAbove();
  
  void insertLayer(LayerIdx);
  void removeLayer(LayerIdx);
  void moveLayerUp(LayerIdx);
  void moveLayerDown(LayerIdx);
  
  void addFrame();
  void addNullFrame();
  void removeFrame();
  
  void requestCell();
  void toggleAnimation();
  void setAnimDelay(int);

public:
  void initLayer();
  void initCell();

  LayerCellsWidget *appendLayer();
  LayerCellsWidget *getLayer(LayerIdx);
  void appendFrame();
  LayerIdx layerCount() const;
  LayerIdx currLayer() const;
  
  void serialize(QIODevice *) const;
  void deserialize(QIODevice *);
  
private:
  TimelineWidget *timeline;
  QVBoxLayout *layout;
  std::vector<LayerCellsWidget *> layers;
  CellPos pos = {0, 0};
  CellRect select = {1, 1, 1, 1};
  FrameIdx frameCount = 0;
  QTimer animTimer;
  
  Cell *getCurr();
  Frame getFrame();
  QPoint getPixelPos();
  
  void setSize();
  
  void paintEvent(QPaintEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
};

class CellScrollWidget final : public ScrollAreaWidget {
  Q_OBJECT
  
public:
  explicit CellScrollWidget(QWidget *);

  CellsWidget *setChild(CellsWidget *);

Q_SIGNALS:
  void rightMarginChanged(int);
  void bottomMarginChanged(int);

public Q_SLOTS:
  void contentResized();
  void ensureVisible(QPoint);

private:
  QWidget *rect = nullptr;

  void resizeEvent(QResizeEvent *) override;
};

#endif
