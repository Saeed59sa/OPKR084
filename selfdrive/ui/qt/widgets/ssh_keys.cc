#include <QNetworkReply>
#include <QHBoxLayout>
#include "widgets/input.hpp"
#include "widgets/ssh_keys.hpp"
#include "common/params.h"
#include <QProcess>
#include <QAction>
#include <QMenu>
#include "home.hpp"

SshControl::SshControl() : AbstractControl("SSH 키 설정", "경고: 이렇게 하면 GitHub 설정의 모든 공개 키에 대한 SSH 액세스 권한이 부여됩니다. 사용자 이외의 GitHub 사용자 이름을 입력하지 마십시오. 콤마 직원은 절대 GitHub 사용자 이름을 추가하라는 요청을 하지 않습니다.", "") {

  // setup widget
  hlayout->addStretch(1);

  username_label.setAlignment(Qt::AlignVCenter);
  username_label.setStyleSheet("color: #aaaaaa");
  hlayout->addWidget(&username_label);

  btn.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btn.setFixedSize(250, 100);
  hlayout->addWidget(&btn);

  QObject::connect(&btn, &QPushButton::released, [=]() {
    if (btn.text() == "설정") {
      username = InputDialog::getText("GitHub 아이디를 입력하세요");
      if (username.length() > 0) {
        btn.setText("로딩중");
        btn.setEnabled(false);
        getUserKeys(username);
      }
    } else {
      Params().remove("GithubUsername");
      Params().remove("GithubSshKeys");
      refresh();
    }
  });

  // setup networking
  manager = new QNetworkAccessManager(this);
  networkTimer = new QTimer(this);
  networkTimer->setSingleShot(true);
  networkTimer->setInterval(5000);
  connect(networkTimer, SIGNAL(timeout()), this, SLOT(timeout()));

  refresh();
}

void SshControl::refresh() {
  QString param = QString::fromStdString(Params().get("GithubSshKeys"));
  if (param.length()) {
    username_label.setText(QString::fromStdString(Params().get("GithubUsername")));
    btn.setText("제거");
  } else {
    username_label.setText("");
    btn.setText("설정");
  }
  btn.setEnabled(true);
}

void SshControl::getUserKeys(QString username){
  QString url = "https://github.com/" + username + ".keys";

  QNetworkRequest request;
  request.setUrl(QUrl(url));
#ifdef QCOM
  QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
  ssl.setCaCertificates(QSslCertificate::fromPath("/usr/etc/tls/cert.pem",
                        QSsl::Pem, QRegExp::Wildcard));
  request.setSslConfiguration(ssl);
#endif

  reply = manager->get(request);
  connect(reply, SIGNAL(finished()), this, SLOT(parseResponse()));
  networkTimer->start();
}

void SshControl::timeout(){
  reply->abort();
}

void SshControl::parseResponse(){
  QString err = "";
  if (reply->error() != QNetworkReply::OperationCanceledError) {
    networkTimer->stop();
    QString response = reply->readAll();
    if (reply->error() == QNetworkReply::NoError && response.length()) {
      Params().put("GithubUsername", username.toStdString());
      Params().put("GithubSshKeys", response.toStdString());
    } else if(reply->error() == QNetworkReply::NoError){
      err = username + " 사용자에 대한 Github 키가 존재하지 않습니다.";
    } else {
      err = username + " 사용자가 Github에 존재하지 않습니다.";
    }
  } else {
    err = "요청된 시간이 초과되었습니다.";
  }

  if (err.length()) {
    ConfirmationDialog::alert(err);
  }

  refresh();
  reply->deleteLater();
  reply = nullptr;
}

OpenpilotView::OpenpilotView() : AbstractControl("오픈파일럿 주행화면 미리보기", "오픈파일럿 주행화면을 미리보기 합니다.", "") {

  // setup widget
  hlayout->addStretch(1);

  btn.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btn.setFixedSize(250, 100);
  hlayout->addWidget(&btn);

  QObject::connect(&btn, &QPushButton::released, [=]() {
    bool stat = Params().getBool("IsOpenpilotViewEnabled");
    if (stat) {
      Params().putBool("IsOpenpilotViewEnabled", false);
    } else {
      Params().putBool("IsOpenpilotViewEnabled", true);
    }
    refresh();
  });
  refresh();
}

void OpenpilotView::refresh() {
  bool param = Params().getBool("IsOpenpilotViewEnabled");
  if (param) {
    btn.setText("미리보기해제");
  } else {
    btn.setText("미리보기");
  }
}

CarRecognition::CarRecognition() : AbstractControl("차량강제인식", "핑거프린트 문제로 차량인식이 안될경우 차량을 선택하여 강제 인식합니다.", "") {

  // setup widget
  hlayout->addStretch(1);
  
  //carname_label.setAlignment(Qt::AlignVCenter);
  carname_label.setStyleSheet("color: #aaaaaa");
  hlayout->addWidget(&carname_label);
  QMenu *vehicle_select_menu = new QMenu();
  vehicle_select_menu->addAction("GENESIS", [=]() {carname = "GENESIS";});
  vehicle_select_menu->addAction("GENESIS_G70", [=]() {carname = "GENESIS_G70";});
  vehicle_select_menu->addAction("GENESIS_G80", [=]() {carname = "GENESIS_G80";});
  vehicle_select_menu->addAction("GENESIS_G90", [=]() {carname = "GENESIS_G90";});
  vehicle_select_menu->addAction("AVANTE", [=]() {carname = "AVANTE";});
  vehicle_select_menu->addAction("I30", [=]() {carname = "I30";});
  vehicle_select_menu->addAction("SONATA", [=]() {carname = "SONATA";});
  vehicle_select_menu->addAction("SONATA_HEV", [=]() {carname = "SONATA_HEV";});
  vehicle_select_menu->addAction("SONATA19", [=]() {carname = "SONATA19";});
  vehicle_select_menu->addAction("SONATA19_HEV", [=]() {carname = "SONATA19_HEV";});
  vehicle_select_menu->addAction("KONA", [=]() {carname = "KONA";});
  vehicle_select_menu->addAction("KONA_EV", [=]() {carname = "KONA_EV";});
  vehicle_select_menu->addAction("KONA_HEV", [=]() {carname = "KONA_HEV";});
  vehicle_select_menu->addAction("IONIQ_EV", [=]() {carname = "IONIQ_EV";});
  vehicle_select_menu->addAction("IONIQ_HEV", [=]() {carname = "IONIQ_HEV";});
  vehicle_select_menu->addAction("SANTA_FE", [=]() {carname = "SANTA_FE";});
  vehicle_select_menu->addAction("PALISADE", [=]() {carname = "PALISADE";});
  vehicle_select_menu->addAction("VELOSTER", [=]() {carname = "VELOSTER";});
  vehicle_select_menu->addAction("GRANDEUR", [=]() {carname = "GRANDEUR";});
  vehicle_select_menu->addAction("GRANDEUR_HEV", [=]() {carname = "GRANDEUR_HEV";});
  vehicle_select_menu->addAction("NEXO", [=]() {carname = "NEXO";});
  vehicle_select_menu->addAction("K3", [=]() {carname = "K3";});
  vehicle_select_menu->addAction("K5", [=]() {carname = "K5";});
  vehicle_select_menu->addAction("K5_HEV", [=]() {carname = "K5_HEV";});
  vehicle_select_menu->addAction("K7", [=]() {carname = "K7";});
  vehicle_select_menu->addAction("K7_HEV", [=]() {carname = "K7_HEV";});
  vehicle_select_menu->addAction("SPORTAGE", [=]() {carname = "SPORTAGE";});
  vehicle_select_menu->addAction("SORENTO", [=]() {carname = "SORENTO";});
  vehicle_select_menu->addAction("STINGER", [=]() {carname = "STINGER";});
  vehicle_select_menu->addAction("NIRO_EV", [=]() {carname = "NIRO_EV";});
  vehicle_select_menu->addAction("NIRO_HEV", [=]() {carname = "NIRO_HEV";});
  vehicle_select_menu->addAction("CEED", [=]() {carname = "CEED";});
  vehicle_select_menu->addAction("SELTOS", [=]() {carname = "SELTOS";});

  QPushButton *set_vehicle_btn = new QPushButton("차량선택");
  set_vehicle_btn->setMenu(vehicle_select_menu);
  hlayout->addWidget(set_vehicle_btn);

  btn.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btn.setFixedSize(200, 100);
  hlayout->addWidget(&btn);

  QObject::connect(&btn, &QPushButton::released, [=]() {
    if (btn.text() == "설정" && carname.length()) {
      Params().put("CarModel", carname.toStdString());
      Params().put("CarModelAbb", carname.toStdString());
      QProcess::execute("/data/openpilot/car_force_set.sh");
      refresh(carname);
    } else {
      carname = "";
      //Params().put("CarModel", "");
      Params().remove("CarModel");
      Params().remove("CarModelAbb");
      refresh(carname);
    }
  });
  refresh(carname);
}

void CarRecognition::refresh(QString carname) {
  QString param = QString::fromStdString(Params().get("CarModelAbb"));
  if (carname.length()) {
    carname_label.setText(carname);
    btn.setText("제거");
  } else if (param.length()) {
    carname_label.setText(QString::fromStdString(Params().get("CarModelAbb")));
    btn.setText("제거");
  } else {
    carname_label.setText("");
    btn.setText("설정");
  }
}

CarForceSet::CarForceSet() : AbstractControl("차량강제인식", "핑거프린트 문제로 차량인식이 안될경우 차량명을 입력하시면 강제 인식 합니다.\n\n입력방법) 아래 참조하여 대문자로 차량명만 입력\nGENESIS, GENESIS_G70, GENESIS_G80, GENESIS_G90, AVANTE, I30, SONATA, SONATA_HEV, SONATA19, SONATA19_HEV, KONA, KONA_EV, KONA_HEV, IONIQ_EV, IONIQ_HEV, SANTA_FE, PALISADE, VELOSTER, GRANDEUR, GRANDEUR_HEV, NEXO, K3, K5, K5_HEV, SPORTAGE, SORENTO, STINGER, NIRO_EV, NIRO_HEV, CEED, K7, K7_HEV, SELTOS", "../assets/offroad/icon_shell.png") {

  // setup widget
  //hlayout->addStretch(1);
  
  //carname_label.setAlignment(Qt::AlignVCenter);
  //carname_label.setStyleSheet("color: #aaaaaa");
  //hlayout->addWidget(&carname_label);

  btnc.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnc.setFixedSize(250, 100);
  hlayout->addWidget(&btnc);

  QObject::connect(&btnc, &QPushButton::released, [=]() {
    if (btnc.text() == "설정") {
      carname = InputDialog::getText("차량명은 이전메뉴 차량강제인식을 클릭하여 확인");
      if (carname.length() > 0) {
        btnc.setText("완료");
        btnc.setEnabled(false);
        Params().put("CarModel", carname.toStdString());
        QProcess::execute("/data/openpilot/car_force_set.sh");
      }
    } else {
      Params().remove("CarModel");
      refreshc();
    }
  });

  refreshc();
}

void CarForceSet::refreshc() {
  QString paramc = QString::fromStdString(Params().get("CarModel"));
  if (paramc.length()) {
    //carname_label.setText(QString::fromStdString(Params().get("CarModel")));
    btnc.setText("제거");
  } else {
    //carname_label.setText("");
    btnc.setText("설정");
  }
  btnc.setEnabled(true);
}


//UI
AutoShutdown::AutoShutdown() : AbstractControl("EON 자동 종료", "운행종료 후 설정시간 이후에 자동으로 이온이 꺼집니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrAutoShutdown"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrAutoShutdown", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrAutoShutdown"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 10 ) {
      value = 10;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrAutoShutdown", values.toStdString());
    refresh();
  });
  refresh();
}

void AutoShutdown::refresh() {
  QString option = QString::fromStdString(Params().get("OpkrAutoShutdown"));
  if (option == "0") {
    label.setText(QString::fromStdString("항상켜기"));
  } else if (option == "1") {
    label.setText(QString::fromStdString("바로끄기"));
  } else if (option == "2") {
    label.setText(QString::fromStdString("30초"));
  } else if (option == "3") {
    label.setText(QString::fromStdString("1분"));
  } else if (option == "4") {
    label.setText(QString::fromStdString("3분"));
  } else if (option == "5") {
    label.setText(QString::fromStdString("5분"));
  } else if (option == "6") {
    label.setText(QString::fromStdString("10분"));
  } else if (option == "7") {
    label.setText(QString::fromStdString("30분"));
  } else if (option == "8") {
    label.setText(QString::fromStdString("1시간"));
  } else if (option == "9") {
    label.setText(QString::fromStdString("3시간"));
  } else if (option == "10") {
    label.setText(QString::fromStdString("5시간"));
  }
  btnminus.setText("－");
  btnplus.setText("＋");
}

VolumeControl::VolumeControl() : AbstractControl("EON 볼륨 조절(%)", "EON의 볼륨을 조절합니다. 안드로이드 기본값/수동설정", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrUIVolumeBoost"));
    int value = str.toInt();
    value = value - 10;
    if (value <= -10 ) {
      value = -10;
    } else {
    }
    QString values = QString::number(value);
    GLWindow::ui_state.nOpkrUIVolumeBoost = value;
    Params().put("OpkrUIVolumeBoost", values.toStdString());
    refresh();
    GLWindow::ui_state.sound->play(AudibleAlert::CHIME_WARNING1);
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrUIVolumeBoost"));
    int value = str.toInt();
    value = value + 10;
    if (value >= 100 ) {
      value = 100;
    } else {
    }
    QString values = QString::number(value);
    GLWindow::ui_state.nOpkrUIVolumeBoost = value;
    Params().put("OpkrUIVolumeBoost", values.toStdString());
    refresh();
    GLWindow::ui_state.sound->play(AudibleAlert::CHIME_WARNING1);
  });
  refresh();
}

void VolumeControl::refresh() {
  QString option = QString::fromStdString(Params().get("OpkrUIVolumeBoost"));
  if (option == "0") {
    label.setText(QString::fromStdString("기본값"));
  } else if (option == "-10") {
    label.setText(QString::fromStdString("음소거"));
  } else {
    label.setText(QString::fromStdString(Params().get("OpkrUIVolumeBoost")));
  }
  btnminus.setText("－");
  btnplus.setText("＋");
}

BrightnessControl::BrightnessControl() : AbstractControl("EON 밝기 조절(%)", "EON화면의 밝기를 조절합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrUIBrightness"));
    int value = str.toInt();
    value = value - 5;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    GLWindow::ui_state.nOpkrUIBrightness = value;
    Params().put("OpkrUIBrightness", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrUIBrightness"));
    int value = str.toInt();
    value = value + 5;
    if (value >= 100 ) {
      value = 100;
    } else {
    }
    QString values = QString::number(value);
    GLWindow::ui_state.nOpkrUIBrightness = value;
    Params().put("OpkrUIBrightness", values.toStdString());
    refresh();
  });
  refresh();
}

void BrightnessControl::refresh() {
  QString option = QString::fromStdString(Params().get("OpkrUIBrightness"));
  if (option == "0") {
    label.setText(QString::fromStdString("자동조절"));
  } else {
    label.setText(QString::fromStdString(Params().get("OpkrUIBrightness")));
  }
  btnminus.setText("－");
  btnplus.setText("＋");
}

ChargingMin::ChargingMin() : AbstractControl("배터리 최소 충전 값", "배터리 최소 충전값을 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrBatteryChargingMin"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 10 ) {
      value = 10;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrBatteryChargingMin", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrBatteryChargingMin"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 90 ) {
      value = 90;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrBatteryChargingMin", values.toStdString());
    refresh();
  });
  refresh();
}

void ChargingMin::refresh() {
  label.setText(QString::fromStdString(Params().get("OpkrBatteryChargingMin")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

ChargingMax::ChargingMax() : AbstractControl("배터리 최대 충전 값", "배터리 최대 충전값을 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrBatteryChargingMax"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 10 ) {
      value = 10;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrBatteryChargingMax", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrBatteryChargingMax"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 90 ) {
      value = 90;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrBatteryChargingMax", values.toStdString());
    refresh();
  });
  refresh();
}

void ChargingMax::refresh() {
  label.setText(QString::fromStdString(Params().get("OpkrBatteryChargingMax")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

RecordCount::RecordCount() : AbstractControl("녹화파일 최대 개수 설정", "녹화 파일 최대 개수를 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("RecordingCount"));
    int value = str.toInt();
    value = value - 5;
    if (value <= 5 ) {
      value = 5;
    } else {
    }
    QString values = QString::number(value);
    Params().put("RecordingCount", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("RecordingCount"));
    int value = str.toInt();
    value = value + 5;
    if (value >= 300 ) {
      value = 300;
    } else {
    }
    QString values = QString::number(value);
    Params().put("RecordingCount", values.toStdString());
    refresh();
  });
  refresh();
}

void RecordCount::refresh() {
  label.setText(QString::fromStdString(Params().get("RecordingCount")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

RecordQuality::RecordQuality() : AbstractControl("녹화 화질 설정", "녹화 화질을 설정합니다. 저화질/중화질/고화질/초고화질", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("RecordingQuality"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("RecordingQuality", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("RecordingQuality"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 3 ) {
      value = 3;
    } else {
    }
    QString values = QString::number(value);
    Params().put("RecordingQuality", values.toStdString());
    refresh();
  });
  refresh();
}

void RecordQuality::refresh() {
  QString option = QString::fromStdString(Params().get("RecordingQuality"));
  if (option == "0") {
    label.setText(QString::fromStdString("저화질"));
  } else if (option == "1") {
    label.setText(QString::fromStdString("중화질"));
  } else if (option == "2") {
    label.setText(QString::fromStdString("고화질"));
  } else {
    label.setText(QString::fromStdString("초고화질"));
  }
  btnminus.setText("◀");
  btnplus.setText("▶");
}

//주행
VariableCruiseProfile::VariableCruiseProfile() : AbstractControl("크루즈 가감속 프로파일", "크루즈 가감속 프로파일을 설정합니다. follow/relaxed", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrVariableCruiseProfile"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrVariableCruiseProfile", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrVariableCruiseProfile"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrVariableCruiseProfile", values.toStdString());
    refresh();
  });
  refresh();
}

void VariableCruiseProfile::refresh() {
  QString option = QString::fromStdString(Params().get("OpkrVariableCruiseProfile"));
  if (option == "0") {
    label.setText(QString::fromStdString("follow"));
  } else {
    label.setText(QString::fromStdString("relaxed"));
  }
  btnminus.setText("◀");
  btnplus.setText("▶");
}

CruisemodeSelInit::CruisemodeSelInit() : AbstractControl("크루즈 시작모드 설정", "크루즈 시작모드를 설정합니다. 오파모드/차간+커브/차간Only/편도1차선", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("CruiseStatemodeSelInit"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("CruiseStatemodeSelInit", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("CruiseStatemodeSelInit"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 3 ) {
      value = 3;
    } else {
    }
    QString values = QString::number(value);
    Params().put("CruiseStatemodeSelInit", values.toStdString());
    refresh();
  });
  refresh();
}

void CruisemodeSelInit::refresh() {
  QString option = QString::fromStdString(Params().get("CruiseStatemodeSelInit"));
  if (option == "0") {
    label.setText(QString::fromStdString("오파모드"));
  } else if (option == "1") {
    label.setText(QString::fromStdString("차간+커브"));
  } else if (option == "2") {
    label.setText(QString::fromStdString("차간Only"));
  } else {
    label.setText(QString::fromStdString("편도1차선"));
  }
  btnminus.setText("◀");
  btnplus.setText("▶");
}

LaneChangeSpeed::LaneChangeSpeed() : AbstractControl("차선변경 속도 설정", "차선변경 가능 속도를 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrLaneChangeSpeed"));
    int value = str.toInt();
    value = value - 5;
    if (value <= 30 ) {
      value = 30;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrLaneChangeSpeed", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrLaneChangeSpeed"));
    int value = str.toInt();
    value = value + 5;
    if (value >= 160 ) {
      value = 160;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrLaneChangeSpeed", values.toStdString());
    refresh();
  });
  refresh();
}

void LaneChangeSpeed::refresh() {
  label.setText(QString::fromStdString(Params().get("OpkrLaneChangeSpeed")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

LaneChangeDelay::LaneChangeDelay() : AbstractControl("차선변경 지연시간 설정", "턴시그널 작동후 차선변경전 지연시간을 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrAutoLaneChangeDelay"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrAutoLaneChangeDelay", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrAutoLaneChangeDelay"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 5 ) {
      value = 5;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrAutoLaneChangeDelay", values.toStdString());
    refresh();
  });
  refresh();
}

void LaneChangeDelay::refresh() {
  QString option = QString::fromStdString(Params().get("OpkrAutoLaneChangeDelay"));
  if (option == "0") {
    label.setText(QString::fromStdString("수동"));
  } else if (option == "1") {
    label.setText(QString::fromStdString("즉시"));
  } else if (option == "2") {
    label.setText(QString::fromStdString("0.5초"));
  } else if (option == "3") {
    label.setText(QString::fromStdString("1초"));
  } else if (option == "4") {
    label.setText(QString::fromStdString("1.5초"));
  } else {
    label.setText(QString::fromStdString("2초"));
  }
  btnminus.setText("－");
  btnplus.setText("＋");
}

LeftCurvOffset::LeftCurvOffset() : AbstractControl("오프셋조정(왼쪽 커브)", "커브구간에서 차량위치를 조정합니다.(-값: 차를 왼쪽으로 이동, +값:차를 오른쪽으로 이동)", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LeftCurvOffsetAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= -30 ) {
      value = -30;
    } else {
    }
    QString values = QString::number(value);
    Params().put("LeftCurvOffsetAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LeftCurvOffsetAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 30 ) {
      value = 30;
    } else {
    }
    QString values = QString::number(value);
    Params().put("LeftCurvOffsetAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void LeftCurvOffset::refresh() {
  label.setText(QString::fromStdString(Params().get("LeftCurvOffsetAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

RightCurvOffset::RightCurvOffset() : AbstractControl("오프셋조정(오른쪽 커브)", "커브구간에서 차량위치를 조정합니다.(-값: 차를 왼쪽으로 이동, +값:차를 오른쪽으로 이동)", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("RightCurvOffsetAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= -30 ) {
      value = -30;
    } else {
    }
    QString values = QString::number(value);
    Params().put("RightCurvOffsetAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("RightCurvOffsetAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 30 ) {
      value = 30;
    } else {
    }
    QString values = QString::number(value);
    Params().put("RightCurvOffsetAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void RightCurvOffset::refresh() {
  label.setText(QString::fromStdString(Params().get("RightCurvOffsetAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

MaxAngleLimit::MaxAngleLimit() : AbstractControl("최대 조향각 설정(각도)", "오파 가능한 핸들의 최대 조향각을 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrMaxAngleLimit"));
    int value = str.toInt();
    value = value - 10;
    if (value <= 80 ) {
      value = 80;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrMaxAngleLimit", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrMaxAngleLimit"));
    int value = str.toInt();
    value = value + 10;
    if (value >= 360 ) {
      value = 360;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrMaxAngleLimit", values.toStdString());
    refresh();
  });
  refresh();
}

void MaxAngleLimit::refresh() {
  QString option = QString::fromStdString(Params().get("OpkrMaxAngleLimit"));
  if (option == "80") {
    label.setText(QString::fromStdString("제한없음"));
  } else {
    label.setText(QString::fromStdString(Params().get("OpkrMaxAngleLimit")));
  }
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerAngleCorrection::SteerAngleCorrection() : AbstractControl("스티어앵글 영점 조정", "직선주로에서 현재조향각이 0이 아닐겨우 SteerAngle 영점을 조정하여 0으로 맞춥니다. ex) 직선주로시 0.5도 인경우, 0.5로 세팅, -0.5도인경우 -0.5로 세팅", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrSteerAngleCorrection"));
    int value = str.toInt();
    value = value - 1;
    if (value <= -50 ) {
      value = -50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrSteerAngleCorrection", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrSteerAngleCorrection"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 50 ) {
      value = 50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OpkrSteerAngleCorrection", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerAngleCorrection::refresh() {
  auto strs = QString::fromStdString(Params().get("OpkrSteerAngleCorrection"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SpeedLimitOffset::SpeedLimitOffset() : AbstractControl("MAP기반 제한속도 오프셋(%)", "맵기반 감속시 GPS속도와 실속도차이를 보상하여 감속합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrSpeedLimitOffset"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    GLWindow::ui_state.speed_lim_off = value;
    Params().put("OpkrSpeedLimitOffset", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OpkrSpeedLimitOffset"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 10 ) {
      value = 10;
    } else {
    }
    QString values = QString::number(value);
    GLWindow::ui_state.speed_lim_off = value;
    Params().put("OpkrSpeedLimitOffset", values.toStdString());
    refresh();
  });
  refresh();
}

void SpeedLimitOffset::refresh() {
  label.setText(QString::fromStdString(Params().get("OpkrSpeedLimitOffset")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

MaxSteer::MaxSteer() : AbstractControl("MAX_STEER", "판다 MAX_STEER 값을 수정합니다. 적용하려면 아래 실행 버튼을 누르세요.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxSteer"));
    int value = str.toInt();
    value = value - 2;
    if (value <= 384 ) {
      value = 384;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxSteer", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxSteer"));
    int value = str.toInt();
    value = value + 2;
    if (value >= 1000 ) {
      value = 1000;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxSteer", values.toStdString());
    refresh();
  });
  refresh();
}

void MaxSteer::refresh() {
  label.setText(QString::fromStdString(Params().get("MaxSteer")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

MaxRTDelta::MaxRTDelta() : AbstractControl("RT_DELTA", "판다 RT_DELTA 값을 수정합니다. 적용하려면 아래 실행 버튼을 누르세요.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxRTDelta"));
    int value = str.toInt();
    value = value - 2;
    if (value <= 50 ) {
      value = 50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxRTDelta", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxRTDelta"));
    int value = str.toInt();
    value = value + 2;
    if (value >= 500 ) {
      value = 500;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxRTDelta", values.toStdString());
    refresh();
  });
  refresh();
}

void MaxRTDelta::refresh() {
  label.setText(QString::fromStdString(Params().get("MaxRTDelta")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

MaxRateUp::MaxRateUp() : AbstractControl("MAX_RATE_UP", "판다 MAX_RATE_UP 값을 수정합니다. 적용하려면 아래 실행 버튼을 누르세요.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxRateUp"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 3 ) {
      value = 3;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxRateUp", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxRateUp"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 7 ) {
      value = 7;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxRateUp", values.toStdString());
    refresh();
  });
  refresh();
}

void MaxRateUp::refresh() {
  label.setText(QString::fromStdString(Params().get("MaxRateUp")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

MaxRateDown::MaxRateDown() : AbstractControl("MAX_RATE_DOWN", "판다 MAX_RATE_DOWN 값을 수정합니다. 적용하려면 아래 실행 버튼을 누르세요.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxRateDown"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 7 ) {
      value = 7;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxRateDown", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MaxRateDown"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 15 ) {
      value = 15;
    } else {
    }
    QString values = QString::number(value);
    Params().put("MaxRateDown", values.toStdString());
    refresh();
  });
  refresh();
}

void MaxRateDown::refresh() {
  label.setText(QString::fromStdString(Params().get("MaxRateDown")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

//튜닝
CameraOffset::CameraOffset() : AbstractControl("CameraOffset", "CameraOffset값을 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("CameraOffsetAdj"));
    int value = str.toInt();
    value = value - 5;
    if (value <= -300 ) {
      value = -300;
    } else {
    }
    QString values = QString::number(value);
    Params().put("CameraOffsetAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("CameraOffsetAdj"));
    int value = str.toInt();
    value = value + 5;
    if (value >= 300 ) {
      value = 300;
    } else {
    }
    QString values = QString::number(value);
    Params().put("CameraOffsetAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void CameraOffset::refresh() {
  auto strs = QString::fromStdString(Params().get("CameraOffsetAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.001;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SRBaseControl::SRBaseControl() : AbstractControl("SteerRatio", "SteerRatio 기본값을 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerRatioAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 80 ) {
      value = 80;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerRatioAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerRatioAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 200) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerRatioAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SRBaseControl::refresh() {
  auto strs = QString::fromStdString(Params().get("SteerRatioAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SRMaxControl::SRMaxControl() : AbstractControl("SteerRatioMax", "SteerRatio 최대값을 설정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerRatioMaxAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 100 ) {
      value = 100;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerRatioMaxAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerRatioMaxAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 250 ) {
      value = 250;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerRatioMaxAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SRMaxControl::refresh() {
  auto strs = QString::fromStdString(Params().get("SteerRatioMaxAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerActuatorDelay::SteerActuatorDelay() : AbstractControl("SteerActuatorDelay", "SteerActuatorDelay값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerActuatorDelayAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerActuatorDelayAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerActuatorDelayAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 100 ) {
      value = 100;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerActuatorDelayAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerActuatorDelay::refresh() {
  auto strs = QString::fromStdString(Params().get("SteerActuatorDelayAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.01;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerRateCost::SteerRateCost() : AbstractControl("SteerRateCost", "SteerRateCost값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerRateCostAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerRateCostAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerRateCostAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 200 ) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerRateCostAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerRateCost::refresh() {
  auto strs = QString::fromStdString(Params().get("SteerRateCostAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.01;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerLimitTimer::SteerLimitTimer() : AbstractControl("SteerLimitTimer", "SteerLimitTimer값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerLimitTimerAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerLimitTimerAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerLimitTimerAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 300 ) {
      value = 300;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerLimitTimerAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerLimitTimer::refresh() {
  auto strs = QString::fromStdString(Params().get("SteerLimitTimerAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.01;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

TireStiffnessFactor::TireStiffnessFactor() : AbstractControl("TireStiffnessFactor", "TireStiffnessFactor값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("TireStiffnessFactorAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("TireStiffnessFactorAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("TireStiffnessFactorAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 200 ) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("TireStiffnessFactorAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void TireStiffnessFactor::refresh() {
  auto strs = QString::fromStdString(Params().get("TireStiffnessFactorAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.01;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerMaxBase::SteerMaxBase() : AbstractControl("SteerMax기본값", "SteerMax기본값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerMaxBaseAdj"));
    int value = str.toInt();
    value = value - 2;
    if (value <= 200 ) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerMaxBaseAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerMaxBaseAdj"));
    int value = str.toInt();
    value = value + 2;
    if (value >= 384 ) {
      value = 384;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerMaxBaseAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerMaxBase::refresh() {
  label.setText(QString::fromStdString(Params().get("SteerMaxBaseAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerMaxMax::SteerMaxMax() : AbstractControl("SteerMax최대값", "SteerMax최대값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerMaxAdj"));
    int value = str.toInt();
    value = value - 2;
    if (value <= 254 ) {
      value = 254;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerMaxAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerMaxAdj"));
    int value = str.toInt();
    value = value + 2;
    if (value >= 1000 ) {
      value = 1000;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerMaxAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerMaxMax::refresh() {
  label.setText(QString::fromStdString(Params().get("SteerMaxAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerMaxv::SteerMaxv() : AbstractControl("SteerMaxV", "SteerMaxV값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerMaxvAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 10 ) {
      value = 10;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerMaxvAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerMaxvAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 30 ) {
      value = 30;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerMaxvAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerMaxv::refresh() {
  auto strs = QString::fromStdString(Params().get("SteerMaxvAdj"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerDeltaUpBase::SteerDeltaUpBase() : AbstractControl("SteerDeltaUp기본값", "SteerDeltaUp기본값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaUpBaseAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 2 ) {
      value = 2;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaUpBaseAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaUpBaseAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 7 ) {
      value = 7;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaUpBaseAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerDeltaUpBase::refresh() {
  label.setText(QString::fromStdString(Params().get("SteerDeltaUpBaseAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerDeltaUpMax::SteerDeltaUpMax() : AbstractControl("SteerDeltaUp최대값", "SteerDeltaUp최대값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaUpAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 3 ) {
      value = 3;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaUpAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaUpAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 7 ) {
      value = 7;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaUpAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerDeltaUpMax::refresh() {
  label.setText(QString::fromStdString(Params().get("SteerDeltaUpAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerDeltaDownBase::SteerDeltaDownBase() : AbstractControl("SteerDeltaDown기본값", "SteerDeltaDown기본값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaDownBaseAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 3 ) {
      value = 3;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaDownBaseAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaDownBaseAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 15 ) {
      value = 15;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaDownBaseAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerDeltaDownBase::refresh() {
  label.setText(QString::fromStdString(Params().get("SteerDeltaDownBaseAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerDeltaDownMax::SteerDeltaDownMax() : AbstractControl("SteerDeltaDown최대값", "SteerDeltaDown최대값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaDownAdj"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 7 ) {
      value = 7;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaDownAdj", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerDeltaDownAdj"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 15 ) {
      value = 15;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerDeltaDownAdj", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerDeltaDownMax::refresh() {
  label.setText(QString::fromStdString(Params().get("SteerDeltaDownAdj")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

SteerThreshold::SteerThreshold() : AbstractControl("SteerThreshold", "SteerThreshold값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerThreshold"));
    int value = str.toInt();
    value = value - 10;
    if (value <= 50 ) {
      value = 50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerThreshold", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("SteerThreshold"));
    int value = str.toInt();
    value = value + 10;
    if (value >= 300 ) {
      value = 300;
    } else {
    }
    QString values = QString::number(value);
    Params().put("SteerThreshold", values.toStdString());
    refresh();
  });
  refresh();
}

void SteerThreshold::refresh() {
  label.setText(QString::fromStdString(Params().get("SteerThreshold")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

//제어
LateralControl::LateralControl() : AbstractControl("조향제어", "조향제어 방법을 설정합니다. (PID/INDI/LQR)", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LateralControlMethod"));
    int latcontrol = str.toInt();
    latcontrol = latcontrol - 1;
    if (latcontrol <= 0 ) {
      latcontrol = 0;
    } else {
    }
    QString latcontrols = QString::number(latcontrol);
    Params().put("LateralControlMethod", latcontrols.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LateralControlMethod"));
    int latcontrol = str.toInt();
    latcontrol = latcontrol + 1;
    if (latcontrol >= 2 ) {
      latcontrol = 2;
    } else {
    }
    QString latcontrols = QString::number(latcontrol);
    Params().put("LateralControlMethod", latcontrols.toStdString());
    refresh();
  });
  refresh();
}

void LateralControl::refresh() {
  QString latcontrol = QString::fromStdString(Params().get("LateralControlMethod"));
  if (latcontrol == "0") {
    label.setText(QString::fromStdString("PID"));
  } else if (latcontrol == "1") {
    label.setText(QString::fromStdString("INDI"));
  } else if (latcontrol == "2") {
    label.setText(QString::fromStdString("LQR"));
  }
  btnminus.setText("◀");
  btnplus.setText("▶");
}

PidKp::PidKp() : AbstractControl("Kp", "Kp값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKp"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKp", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKp"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 50 ) {
      value = 50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKp", values.toStdString());
    refresh();
  });
  refresh();
}

void PidKp::refresh() {
  auto strs = QString::fromStdString(Params().get("PidKp"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.01;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

PidKi::PidKi() : AbstractControl("Ki", "Ki값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKi"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKi", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKi"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 100 ) {
      value = 100;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKi", values.toStdString());
    refresh();
  });
  refresh();
}

void PidKi::refresh() {
  auto strs = QString::fromStdString(Params().get("PidKi"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.001;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

PidKd::PidKd() : AbstractControl("Kd", "Kd값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKd"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKd", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKd"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 300 ) {
      value = 300;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKd", values.toStdString());
    refresh();
  });
  refresh();
}

void PidKd::refresh() {
  auto strs = QString::fromStdString(Params().get("PidKd"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.01;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

PidKf::PidKf() : AbstractControl("Kf", "Kf값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKf"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKf", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("PidKf"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 50 ) {
      value = 50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("PidKf", values.toStdString());
    refresh();
  });
  refresh();
}

void PidKf::refresh() {
  auto strs = QString::fromStdString(Params().get("PidKf"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.00001;
  QString valuefs = QString::number(valuef, 'f', 5);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

IgnoreZone::IgnoreZone() : AbstractControl("IgnoreZone", "IgnoreZone값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("IgnoreZone"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    } else {
    }
    QString values = QString::number(value);
    Params().put("IgnoreZone", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("IgnoreZone"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 30 ) {
      value = 30;
    } else {
    }
    QString values = QString::number(value);
    Params().put("IgnoreZone", values.toStdString());
    refresh();
  });
  refresh();
}

void IgnoreZone::refresh() {
  auto strs = QString::fromStdString(Params().get("IgnoreZone"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

OuterLoopGain::OuterLoopGain() : AbstractControl("OuterLoopGain", "OuterLoopGain값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OuterLoopGain"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OuterLoopGain", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("OuterLoopGain"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 200 ) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("OuterLoopGain", values.toStdString());
    refresh();
  });
  refresh();
}

void OuterLoopGain::refresh() {
  auto strs = QString::fromStdString(Params().get("OuterLoopGain"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

InnerLoopGain::InnerLoopGain() : AbstractControl("InnerLoopGain", "InnerLoopGain값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("InnerLoopGain"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("InnerLoopGain", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("InnerLoopGain"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 200 ) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("InnerLoopGain", values.toStdString());
    refresh();
  });
  refresh();
}

void InnerLoopGain::refresh() {
  auto strs = QString::fromStdString(Params().get("InnerLoopGain"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

TimeConstant::TimeConstant() : AbstractControl("TimeConstant", "TimeConstant값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("TimeConstant"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("TimeConstant", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("TimeConstant"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 200 ) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("TimeConstant", values.toStdString());
    refresh();
  });
  refresh();
}

void TimeConstant::refresh() {
  auto strs = QString::fromStdString(Params().get("TimeConstant"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

ActuatorEffectiveness::ActuatorEffectiveness() : AbstractControl("ActuatorEffectiveness", "ActuatorEffectiveness값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("ActuatorEffectiveness"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("ActuatorEffectiveness", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("ActuatorEffectiveness"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 200 ) {
      value = 200;
    } else {
    }
    QString values = QString::number(value);
    Params().put("ActuatorEffectiveness", values.toStdString());
    refresh();
  });
  refresh();
}

void ActuatorEffectiveness::refresh() {
  auto strs = QString::fromStdString(Params().get("ActuatorEffectiveness"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.1;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

Scale::Scale() : AbstractControl("Scale", "Scale값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("Scale"));
    int value = str.toInt();
    value = value - 50;
    if (value <= 50 ) {
      value = 50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("Scale", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("Scale"));
    int value = str.toInt();
    value = value + 50;
    if (value >= 5000 ) {
      value = 5000;
    } else {
    }
    QString values = QString::number(value);
    Params().put("Scale", values.toStdString());
    refresh();
  });
  refresh();
}

void Scale::refresh() {
  label.setText(QString::fromStdString(Params().get("Scale")));
  btnminus.setText("－");
  btnplus.setText("＋");
}

LqrKi::LqrKi() : AbstractControl("LqrKi", "ki값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LqrKi"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("LqrKi", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LqrKi"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 100 ) {
      value = 100;
    } else {
    }
    QString values = QString::number(value);
    Params().put("LqrKi", values.toStdString());
    refresh();
  });
  refresh();
}

void LqrKi::refresh() {
  auto strs = QString::fromStdString(Params().get("LqrKi"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.001;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}

DcGain::DcGain() : AbstractControl("DcGain", "DcGain값을 조정합니다.", "../assets/offroad/icon_shell.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("DcGain"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    } else {
    }
    QString values = QString::number(value);
    Params().put("DcGain", values.toStdString());
    refresh();
  });
  
  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("DcGain"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 50 ) {
      value = 50;
    } else {
    }
    QString values = QString::number(value);
    Params().put("DcGain", values.toStdString());
    refresh();
  });
  refresh();
}

void DcGain::refresh() {
  auto strs = QString::fromStdString(Params().get("DcGain"));
  int valuei = strs.toInt();
  float valuef = valuei * 0.0001;
  QString valuefs = QString::number(valuef);
  label.setText(QString::fromStdString(valuefs.toStdString()));
  btnminus.setText("－");
  btnplus.setText("＋");
}