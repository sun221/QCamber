#include "viewerwindow.h"
#include "ui_viewerwindow.h"

#include <QtGui>
#include <QDebug>

#include "context.h"

extern Context ctx;

ViewerWindow::ViewerWindow(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::ViewerWindow)
{
  ui->setupUi(this);

  loadColorConfig();

  for (int i = 0; i < m_colors.size(); ++i) {
    m_colorsMap[i] = false;
  }

  m_cursorCoordLabel = new QLabel();
  m_featureDetailLabel = new QLabel();
  m_cursorCoordLabel->setAlignment(Qt::AlignRight);
  m_featureDetailLabel->setAlignment(Qt::AlignCenter);
  statusBar()->addPermanentWidget(m_featureDetailLabel);
  statusBar()->addPermanentWidget(m_cursorCoordLabel, 1);

  m_layout = new QVBoxLayout();
  m_tool_layout = new QVBoxLayout();
  ui->scrollWidget->setLayout(m_layout);
  ui->groupBox_Tool->setLayout(m_tool_layout);

  connect(ui->viewWidget->scene(), SIGNAL(mouseMove(QPointF)), this,
      SLOT(updateCursorCoord(QPointF)));
  connect(ui->viewWidget->scene(), SIGNAL(featureSelected(Symbol*)), this,
      SLOT(updateFeatureDetail(Symbol*)));
}

ViewerWindow::~ViewerWindow()
{
  delete ui;
}

void ViewerWindow::addLayerLabel(const QStringList& layerNames)
{
  ui->viewWidget->clear_scene();
  ui->viewWidget->loadProfile(this->windowTitle());

  clearLayout(m_layout, true);
  QString pathTmpl = "steps/%1/layers/%2";

  for(int i = 0; i < layerNames.count(); ++i)
  {
    LayerSelector *layer = new LayerSelector(layerNames[i],
        pathTmpl.arg(this->windowTitle()).arg(layerNames[i]));

    connect(layer, SIGNAL(Clicked(LayerSelector*, bool)), this,
        SLOT(showLayer(LayerSelector*, bool)));
    m_layout->addWidget(layer);
  }
}

void ViewerWindow::clearLayout(QLayout* layout, bool deleteWidgets)
{
  while (QLayoutItem* item = layout->takeAt(0))
  {
    if (deleteWidgets)
    {
      if (QWidget* widget = item->widget())
        delete widget;
    }
    else if (QLayout* childLayout = item->layout())
      clearLayout(childLayout, deleteWidgets);
    delete item;
  }
}


Features* ViewerWindow::makeFeature(QString path, const QPen& pen,
    const QBrush& brush)
{
  Features* features = new Features(ctx.loader->featuresPath(path));
  features->setPen(pen);
  features->setBrush(brush);
  return features;
}

void ViewerWindow::showLayer(LayerSelector* selector, bool selected)
{
  if (!selected) {
    if (!selector->features) {
      selector->features = makeFeature(selector->path(),
          QPen(selector->color(), 0), QBrush(selector->color()));
      ui->viewWidget->addItem(selector->features);
    }
    selector->setColor(nextColor());
    selector->features->setOpacity(1);
    selector->features->setDoComposite(true);
  } else {
    int index = m_colors.indexOf(selector->color());
    m_colorsMap[index] = false;
    selector->features->setDoComposite(false);
    selector->features->setOpacity(0);
  }
}

QColor ViewerWindow::nextColor(void)
{
  for (int i = 0; i < m_colors.size(); ++i) {
    if (!m_colorsMap[i]) {
      m_colorsMap[i] = true;
      return m_colors[i];
    }
  }
  return Qt::red;
}

void ViewerWindow::loadColorConfig()
{
  ctx.bg_color = QColor(ctx.config->value("color/BG").toString());
  ui->viewWidget->setBackgroundColor(ctx.bg_color);

  QString config("color/C%1");
  m_colors.clear();

  for(int i = 1; i < N_COLOR + 1; i++) {
    m_colors << QColor(ctx.config->value(config.arg(i)).toString());
  }

  for (int i = 0; i < m_colors.size(); ++i) {
    m_colorsMap[i] = false;
  }
}

void ViewerWindow::on_actionSetColor_triggered()
{
  ColorSettings dialog;
  dialog.exec();
  loadColorConfig();
}

void ViewerWindow::updateCursorCoord(QPointF pos)
{
  QString text;
  text.sprintf("(%f, %f)", pos.x(), pos.y());
  m_cursorCoordLabel->setText(text);
}

void ViewerWindow::updateFeatureDetail(Symbol* symbol)
{
  m_featureDetailLabel->setText(symbol->name());
}