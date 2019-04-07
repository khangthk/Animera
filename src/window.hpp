//
//  window.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef window_hpp
#define window_hpp

#include "undo object.hpp"
#include "clear object.hpp"
#include "editor widget.hpp"
#include "timeline widget.hpp"
#include "status bar widget.hpp"
#include "tool select widget.hpp"
#include "color picker widget.hpp"
#include <QtWidgets/qmainwindow.h>

class Window final : public QMainWindow {
  Q_OBJECT

public:
  explicit Window(QRect);
  
private:
  QWidget bottom;
  Animation anim;
  UndoObject undo;
  ClearObject clear;
  EditorWidget editor;
  ToolSelectWidget tools;
  TimelineWidget timeline;
  StatusBarWidget statusBar;
  ColorPickerWidget colorPicker;
  QMenuBar *menubar = nullptr;
  
  void setupUI();
  void setupMenubar();
  void makeDockWidget(Qt::DockWidgetArea, QWidget *);
  void connectSignals();
};

#endif
