#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <QProcess>

#ifndef QCOM
#include "networking.hpp"
#endif
#include "settings.hpp"
#include "widgets/input.hpp"
#include "widgets/toggle.hpp"
#include "widgets/offroad_alerts.hpp"
#include "widgets/scrollview.hpp"
#include "widgets/controls.hpp"
#include "widgets/ssh_keys.hpp"
#include "common/params.h"
#include "common/util.h"
#include "selfdrive/hardware/hw.h"
#include "home.hpp"


QWidget * toggles_panel() {
  QVBoxLayout *toggles_list = new QVBoxLayout();

  toggles_list->addWidget(new ParamControl("OpenpilotEnabledToggle",
                                            "استخدام الطيار المفتوح",
                                            "استخدم نظام التوجيه المفتوح للتحكم التكيفي في ثبات السرعة والمساعدة في الحفاظ على المسار.  يتطلب استخدام هذه الميزة الرعاية في جميع الأوقات.  يسري تغيير هذا الإعداد عند إيقاف تشغيل السيارة.",
                                            "../assets/offroad/icon_openpilot.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsLdwEnabled",
                                            "استخدم تحذير مغادرة المسار",
                                            "",
                                            "../assets/offroad/icon_warning.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsRHD",
                                            "استخدام طريقة القيادة اليمنى",
                                            "اسمح للطيار المفتوح بالالتزام بقواعد المرور على اليسار وإجراء مراقبة للسائق من مقعد السائق الأيمن.",
                                            "../assets/offroad/icon_openpilot_mirrored.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsMetric",
                                            "استخدم النظام المتري",
                                            "يعرض السرعة بالكيلومتر / الساعة بدلاً من ميل / الساعة.",
                                            "../assets/offroad/icon_metric.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("CommunityFeaturesToggle",
                                            "커뮤니티 기능 사용",
                                            "comma.ai استخدم ميزات من مجتمع المصادر المفتوحة التي لم يتم صيانتها أو دعمها ولم يتم التحقق منها لتتوافق مع نموذج الأمان القياسي.  تتضمن هذه الميزات المركبات التي تساعد المجتمع والأجهزة المدعومة من المجتمع.  يجب توخي الحذر بشكل خاص عند استخدام هذه الميزات.",
                                            "../assets/offroad/icon_shell.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  ParamControl *record_toggle = new ParamControl("RecordFront",
                                            "تسجيل وتحميل فيديو السائق",
                                            "تحميل البيانات من كاميرات مراقبة السائق وتحسين خوارزميات مراقبة السائق.",
                                            "../assets/offroad/icon_network.png");
  toggles_list->addWidget(record_toggle);
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("EndToEndToggle",
                                           "\U0001f96c وضع تعطيل المسار (Alpha) \U0001f96c",
                                           "في هذا الوضع ، لا يقود الطيار المفتوح الطريق ، بل يركض كما لو كان إنسانًا.",
                                           "../assets/offroad/icon_road.png"));

#ifdef QCOM2
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("EnableWideCamera",
                                           "تفعيل استخدام الكاميرا ذات الزاوية العريضة",
                                           "Use wide angle camera for driving and ui. Only takes effect after reboot.",
                                           "../assets/offroad/icon_openpilot.png"));
#endif
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("OpkrEnableDriverMonitoring",
                                           "باستخدام مراقبة السائق",
                                           "استخدام مراقبة السائق.",
                                           "../assets/offroad/icon_shell.png"));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("OpkrEnableLogger",
                                           "تمكين التسجيل والتحميل",
                                           "بعد تسجيل سجل القيادة ، يتم إرساله إلى خادم الفاصلة..",
                                           "../assets/offroad/icon_shell.png"));

  bool record_lock = Params().getBool("RecordFrontLock");
  record_toggle->setEnabled(!record_lock);

  QWidget *widget = new QWidget;
  widget->setLayout(toggles_list);
  return widget;
}

DevicePanel::DevicePanel(QWidget* parent) : QWidget(parent) {
  QVBoxLayout *device_layout = new QVBoxLayout;

  Params params = Params();

  QString dongle = QString::fromStdString(params.get("DongleId", false));
  device_layout->addWidget(new LabelControl("Dongle ID", dongle));
  device_layout->addWidget(horizontal_line());

  QString serial = QString::fromStdString(params.get("HardwareSerial", false));
  device_layout->addWidget(new LabelControl("Serial", serial));

  // offroad-only buttons
  QList<ButtonControl*> offroad_btns;

  offroad_btns.append(new ButtonControl("فيديو السائق", "معاينة",
                                   "قم بمعاينة كاميرا مراقبة السائق وقم بتحسين موقع تركيب الجهاز لتوفير أفضل تجربة مراقبة للسائق. (يجب إيقاف تشغيل السيارة.)",
                                   [=]() {
                                      Params().putBool("IsDriverViewEnabled", true);
                                      GLWindow::ui_state.scene.driver_view = true; }
                                    ));

  QString resetCalibDesc = "لاستخدام دليل مفتوح ، يجب تركيب الجهاز في حدود 4 درجات إلى اليسار أو اليمين و 5 درجات لأعلى أو لأسفل. تتم معايرة الطيار المفتوح باستمرار ، لذلك ليست هناك حاجة كبيرة لإعادة ضبطه.";
  ButtonControl *resetCalibBtn = new ButtonControl("معلومات المعايرة", "يتأكد", resetCalibDesc, [=]() {
    QString desc = "[ضمن 4 درجات يسار / يمين و 5 درجات أعلى / أسفل]";
    std::string calib_bytes = Params().get("CalibrationParams");
    if (!calib_bytes.empty()) {
      try {
        AlignedBuffer aligned_buf;
        capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
        auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
        if (calib.getCalStatus() != 0) {
          double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
          double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
          desc += QString("\n الجهاز موجود٪ 1 °٪ 2 و٪ 3 °٪ 4.")
                                .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? "فوق" : "تحت",
                                     QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? "إلى اليمين" : "إلى اليسار");
        }
      } catch (kj::Exception) {
        qInfo() << "معلمة المعايرة غير صالحة";
      }
    }
    if (ConfirmationDialog::confirm(desc)) {
      //Params().remove("CalibrationParams");
    }
  });
  connect(resetCalibBtn, &ButtonControl::showDescription, [=]() {
    QString desc = resetCalibDesc;
    std::string calib_bytes = Params().get("CalibrationParams");
    if (!calib_bytes.empty()) {
      try {
        AlignedBuffer aligned_buf;
        capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
        auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
        if (calib.getCalStatus() != 0) {
          double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
          double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
          desc += QString("\n الجهاز موجود٪ 1 °٪ 2 و٪ 3 °٪ 4.")
                                .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? "فوق" : "تحت",
                                     QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? "إلى اليمين" : "إلى اليسار");
        }
      } catch (kj::Exception) {
        qInfo() << "معلمة المعايرة غير صالحة";
      }
    }
    resetCalibBtn->setDescription(desc);
  });
  offroad_btns.append(resetCalibBtn);

  offroad_btns.append(new ButtonControl("شاهد دليل التدريب", "اعادتها",
                                        "تحقق من القواعد والوظائف والقيود الخاصة بالطيارين المفتوحين..", [=]() {
    if (ConfirmationDialog::confirm("هل ترغب في مراجعة دليل التدريب مرة أخرى؟")) {
      Params().remove("CompletedTrainingVersion");
      emit reviewTrainingGuide();
    }
  }));

  QString brand = params.getBool("Passive") ? "داش كام" : "افتح الطيار";
  offroad_btns.append(new ButtonControl(brand + " إزالة", "إزالة", "", [=]() {
    if (ConfirmationDialog::confirm("هل تريد إزالة?")) {
      Params().putBool("DoUninstall", true);
    }
  }));

  for(auto &btn : offroad_btns){
    device_layout->addWidget(horizontal_line());
    QObject::connect(parent, SIGNAL(offroadTransition(bool)), btn, SLOT(setEnabled(bool)));
    device_layout->addWidget(btn);
  }

  device_layout->addWidget(horizontal_line());

  // cal reset and param init buttons
  QHBoxLayout *cal_param_init_layout = new QHBoxLayout();
  cal_param_init_layout->setSpacing(50);

  QPushButton *calinit_btn = new QPushButton("إعادة ضبط المعايرة");
  cal_param_init_layout->addWidget(calinit_btn);
  QObject::connect(calinit_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("هل يجب علي إعادة ضبط المعايرة؟ سيتم إعادة التشغيل تلقائيًا.")) {
      Params().remove("CalibrationParams");
      QProcess::execute("reboot");
    }
  });

  QPushButton *paraminit_btn = new QPushButton("تهيئة المعلمة");
  cal_param_init_layout->addWidget(paraminit_btn);
  QObject::connect(paraminit_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("إعادة المعلمات إلى حالتها الأولية. هل ترغب في المتابعة؟")) {
      QProcess::execute("/data/openpilot/init_param.sh");
    }
  });

  // preset1 buttons
  QHBoxLayout *presetone_layout = new QHBoxLayout();
  presetone_layout->setSpacing(50);

  QPushButton *presetoneload_btn = new QPushButton("تحميل الإعداد المسبق 1");
  presetone_layout->addWidget(presetoneload_btn);
  QObject::connect(presetoneload_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("هل يجب علينا تحميل الإعداد المسبق 1؟")) {
      QProcess::execute("/data/openpilot/load_preset1.sh");
    }
  });

  QPushButton *presetonesave_btn = new QPushButton("حفظ الإعداد المسبق 1");
  presetone_layout->addWidget(presetonesave_btn);
  QObject::connect(presetonesave_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("حفظ الإعداد المسبق 1؟")) {
      QProcess::execute("/data/openpilot/save_preset1.sh");
    }
  });

  // preset2 buttons
  QHBoxLayout *presettwo_layout = new QHBoxLayout();
  presettwo_layout->setSpacing(50);

  QPushButton *presettwoload_btn = new QPushButton("تحميل الإعداد المسبق 2");
  presettwo_layout->addWidget(presettwoload_btn);
  QObject::connect(presettwoload_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("هل يجب علينا تحميل الإعداد المسبق 2؟")) {
      QProcess::execute("/data/openpilot/load_preset2.sh");
    }
  });

  QPushButton *presettwosave_btn = new QPushButton("حفظ الإعداد المسبق 2");
  presettwo_layout->addWidget(presettwosave_btn);
  QObject::connect(presettwosave_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("حفظ الإعداد المسبق 2؟")) {
      QProcess::execute("/data/openpilot/save_preset2.sh");
    }
  });

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(50);

  QPushButton *reboot_btn = new QPushButton("إعادة تشغيل");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("هل تريد البدء من جديد؟")) {
      Hardware::reboot();
    }
  });

  QPushButton *poweroff_btn = new QPushButton("انقطاع التيار الكهربائي");
  poweroff_btn->setStyleSheet("background-color: #E22C2C;");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("هل ترغب في فصل الطاقة؟")) {
      Hardware::poweroff();
    }
  });

  device_layout->addLayout(cal_param_init_layout);

  device_layout->addWidget(horizontal_line());

  device_layout->addLayout(presetone_layout);
  device_layout->addLayout(presettwo_layout);

  device_layout->addWidget(horizontal_line());

  device_layout->addLayout(power_layout);

  setLayout(device_layout);
  setStyleSheet(R"(
    QPushButton {
      padding: 20;
      height: 120px;
      border-radius: 15px;
      background-color: #393939;
    }
  )");
}

DeveloperPanel::DeveloperPanel(QWidget* parent) : QFrame(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  setLayout(main_layout);
  setStyleSheet(R"(QLabel {font-size: 50px;})");
}

void DeveloperPanel::showEvent(QShowEvent *event) {
  Params params = Params();
  std::string brand = params.getBool("Passive") ? "داش كام" : "افتح القيادة الذاتية";
  QList<QPair<QString, std::string>> dev_params = {
    {"Version", brand + " v" + params.get("Version", false).substr(0, 14)},
    {"Git Branch", params.get("GitBranch", false)},
    {"Git Commit", params.get("GitCommit", false).substr(0, 10)},
    {"Panda Firmware", params.get("PandaFirmwareHex", false)},
    {"OS Version", Hardware::get_os_version()},
  };

  for (int i = 0; i < dev_params.size(); i++) {
    const auto &[name, value] = dev_params[i];
    QString val = QString::fromStdString(value).trimmed();
    if (labels.size() > i) {
      labels[i]->setText(val);
    } else {
      labels.push_back(new LabelControl(name, val));
      layout()->addWidget(labels[i]);
      if (i < (dev_params.size() - 1)) {
        layout()->addWidget(horizontal_line());
      }
    }
  }
}

QWidget * network_panel(QWidget * parent) {
#ifdef QCOM
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setSpacing(30);

  layout->addWidget(new OpenpilotView());
  layout->addWidget(horizontal_line());
  // wifi + tethering buttons
  layout->addWidget(new ButtonControl("WiFi ضبط", "الحرارة", "",
                                      [=]() { HardwareEon::launch_wifi(); }));
  layout->addWidget(horizontal_line());

  layout->addWidget(new ButtonControl("إعدادات الربط", "열기", "",
                                      [=]() { HardwareEon::launch_tethering(); }));
  layout->addWidget(horizontal_line());

  // SSH key management
  layout->addWidget(new SshToggle());
  layout->addWidget(horizontal_line());
  layout->addWidget(new SshControl());
  layout->addWidget(horizontal_line());
  layout->addWidget(new SshLegacyToggle());
  layout->addWidget(horizontal_line());

  layout->addWidget(new GitHash());
  const char* gitpull = "/data/openpilot/gitpull.sh ''";
  layout->addWidget(new ButtonControl("Git Pull", "إعدام", "ستنعكس أي تغييرات يتم إجراؤها في Git البعيد محليًا ثم يتم إعادة تشغيلها تلقائيًا. إذا لم تكن هناك تغييرات ، فلا تقم بإعادة التشغيل. إذا تم تغيير الملف المحلي ، فقد لا يتمكن من عكس تفاصيل Git البعيدة. لاحظ من فضلك.",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("يقوم Git بإعادة التشغيل تلقائيًا بعد تطبيق التغييرات. إذا لم يكن كذلك ، فلا تقم بإعادة التشغيل. هل ترغب في المتابعة؟")){
                                          std::system(gitpull);
                                        }
                                      }));

  layout->addWidget(horizontal_line());

  const char* git_reset = "/data/openpilot/git_reset.sh ''";
  layout->addWidget(new ButtonControl("Git Reset", "إعدام", "عد التهيئة القسرية للتغييرات المحلية ، يتم تطبيق أحدث محفوظات الالتزام عن بُعد. يرجى ملاحظة أن التغييرات المحلية ستختفي..",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("بعد التهيئة القسرية للتغييرات المحلية ، يتم تطبيق أحدث محفوظات الالتزام الخاصة بـ RemoteGit. هل ترغب في المتابعة؟")){
                                          std::system(git_reset);
                                        }
                                      }));


  layout->addWidget(horizontal_line());

  const char* gitpull_cancel = "/data/openpilot/gitpull_cancel.sh ''";
  layout->addWidget(new ButtonControl("Git Pull إلغاء", "إعدام", "Git Pullإلغاء والعودة إلى الحالة السابقة. إذا كان هناك العديد من الالتزامات ، فسيتم إعادتها إلى الحالة السابقة مباشرةً قبل الالتزام الأخير..",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("GitPull العودة إلى الحالة السابقة. هل ترغب في المتابعة؟")){
                                          std::system(gitpull_cancel);
                                        }
                                      }));

  layout->addWidget(horizontal_line());

//  const char* param_init = "/data/openpilot/init_param.sh ''";
//  layout->addWidget(new ButtonControl("تهيئة المعلمة", "إعدام", "إعادة المعلمات إلى حالة التثبيت الأصلية.",
//                                      [=]() { 
//                                        if (ConfirmationDialog::confirm("إرجاع المعلمات إلى حالتها الأصلية المثبتة. هل ترغب في المتابعة؟")){
//                                          std::system(param_init);
//                                        }
//                                      }));
//
//  layout->addWidget(horizontal_line());

  const char* panda_flashing = "/data/openpilot/panda_flashing.sh ''";
  layout->addWidget(new ButtonControl(" ", "إعدام", "امض الباندا عندما يكون يومض الباندا قيد التقدم ، يومض مؤشر LED الأخضر للباندا بسرعة ويتم إعادة التشغيل تلقائيًا عند الانتهاء. لا تقم أبدًا بإيقاف تشغيل الجهاز أو إزالته بشكل تعسفي.",
                                      [=]() {
                                        if (ConfirmationDialog::confirm("هل تريد المتابعة مع Panda Flashing؟")) {
                                          std::system(panda_flashing);
                                        }
                                      }));

  layout->addStretch(1);

  QWidget *w = new QWidget;
  w->setLayout(layout);
#else
  Networking *w = new Networking(parent);
#endif
  return w;
}

QWidget * user_panel(QWidget * parent) {
  QVBoxLayout *layout = new QVBoxLayout;

  // OPKR
  layout->addWidget(new LabelControl("UI설정", ""));
  layout->addWidget(new AutoShutdown());
  layout->addWidget(new AutoScreenDimmingToggle());
  layout->addWidget(new VolumeControl());
  layout->addWidget(new BrightnessControl());
  layout->addWidget(new GetoffAlertToggle());
  layout->addWidget(new BatteryChargingControlToggle());
  layout->addWidget(new ChargingMin());
  layout->addWidget(new ChargingMax());
  layout->addWidget(new DrivingRecordToggle());
  layout->addWidget(new HotspotOnBootToggle());
  layout->addWidget(new RecordCount());
  layout->addWidget(new RecordQuality());
  const char* record_del = "rm -f /storage/emulated/0/videos/*";
  layout->addWidget(new ButtonControl("녹화파일 전부 삭제", "실행", "저장된 녹화파일을 모두 삭제합니다.",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("저장된 녹화파일을 모두 삭제합니다. 진행하시겠습니까?")){
                                          std::system(record_del);
                                        }
                                      }));

  layout->addWidget(horizontal_line());
  layout->addWidget(new LabelControl("주행설정", ""));
  layout->addWidget(new AutoResumeToggle());
  layout->addWidget(new VariableCruiseToggle());
  layout->addWidget(new VariableCruiseProfile());
  layout->addWidget(new CruisemodeSelInit());
  layout->addWidget(new LaneChangeSpeed());
  layout->addWidget(new LaneChangeDelay());
  layout->addWidget(new LeftCurvOffset());
  layout->addWidget(new RightCurvOffset());
  layout->addWidget(new BlindSpotDetectToggle());
  layout->addWidget(new MaxAngleLimit());
  layout->addWidget(new SteerAngleCorrection());
  layout->addWidget(new TurnSteeringDisableToggle());
  layout->addWidget(new CruiseOverMaxSpeedToggle());
  layout->addWidget(new SpeedLimitOffset());
  layout->addWidget(new MapDecelOnlyToggle());
  layout->addWidget(new CruiseGapAdjustToggle());
  layout->addWidget(new AutoEnabledToggle());
  layout->addWidget(new CruiseAutoResToggle());
  layout->addWidget(new SteerWindDownToggle());

  layout->addWidget(horizontal_line());
  layout->addWidget(new LabelControl("개발자", ""));
  layout->addWidget(new DebugUiOneToggle());
  layout->addWidget(new DebugUiTwoToggle());
  layout->addWidget(new PrebuiltToggle());
  layout->addWidget(new FPToggle());
  layout->addWidget(new FPTwoToggle());
  layout->addWidget(new LDWSToggle());
  layout->addWidget(new GearDToggle());
  layout->addWidget(new ComIssueToggle());
  const char* cal_ok = "cp -f /data/openpilot/selfdrive/assets/addon/param/CalibrationParams /data/params/d/";
  layout->addWidget(new ButtonControl("캘리브레이션 강제 활성화", "실행", "실주행으로 캘리브레이션을 설정하지 않고 이온을 초기화 한경우 인게이지 확인용도로 캘리브레이션을 강제 설정합니다.",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("캘리브레이션을 강제로 설정합니다. 인게이지 확인용이니 실 주행시에는 초기화 하시기 바랍니다.")){
                                          std::system(cal_ok);
                                        }
                                      }));
  layout->addWidget(horizontal_line());
  layout->addWidget(new CarRecognition());
  //layout->addWidget(new CarForceSet());
  //QString car_model = QString::fromStdString(Params().get("CarModel", false));
  //layout->addWidget(new LabelControl("현재차량모델", ""));
  //layout->addWidget(new LabelControl(car_model, ""));

  layout->addWidget(horizontal_line());
  layout->addWidget(new LabelControl("판다 값", "주의要"));
  layout->addWidget(new MaxSteer());
  layout->addWidget(new MaxRTDelta());
  layout->addWidget(new MaxRateUp());
  layout->addWidget(new MaxRateDown());
  const char* p_edit_go = "/data/openpilot/p_edit.sh ''";
  layout->addWidget(new ButtonControl("판다 값 최적화", "실행", "판다 값을 적정값으로 최적화 합니다.",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("판다 값을 최적화 합니다. 조지 싸장님 쌀랑해요~")){
                                          std::system(p_edit_go);
                                        }
                                      }));
  const char* m_edit_go = "/data/openpilot/m_edit.sh ''";
  layout->addWidget(new ButtonControl("모니터링 최적화", "실행", "야간 감시 및 터널 안 원활한 모니터링을 위하여 파라미터를 수정합니다.",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("야간감시 및 터널 모니터링을 최적화 합니다. 싸장님 좋아요. 콤마 쌀랑해요~")){
                                          std::system(m_edit_go);
                                        }
                                      }));
  //layout->addWidget(horizontal_line());

  layout->addStretch(1);

  QWidget *w = new QWidget;
  w->setLayout(layout);

  return w;
}

QWidget * tuning_panel(QWidget * parent) {
  QVBoxLayout *layout = new QVBoxLayout;

  // OPKR
  layout->addWidget(new LabelControl("튜닝메뉴", ""));
  layout->addWidget(new CameraOffset());
  layout->addWidget(new LiveSteerRatioToggle());
  layout->addWidget(new SRBaseControl());
  layout->addWidget(new SRMaxControl());
  layout->addWidget(new SteerActuatorDelay());
  layout->addWidget(new SteerRateCost());
  layout->addWidget(new SteerLimitTimer());
  layout->addWidget(new TireStiffnessFactor());
  layout->addWidget(new SteerMaxBase());
  layout->addWidget(new SteerMaxMax());
  layout->addWidget(new SteerMaxv());
  layout->addWidget(new VariableSteerMaxToggle());
  layout->addWidget(new SteerDeltaUpBase());
  layout->addWidget(new SteerDeltaUpMax());
  layout->addWidget(new SteerDeltaDownBase());
  layout->addWidget(new SteerDeltaDownMax());
  layout->addWidget(new VariableSteerDeltaToggle());
  layout->addWidget(new SteerThreshold());

  layout->addWidget(horizontal_line());

  layout->addWidget(new LabelControl("제어메뉴", ""));
  layout->addWidget(new LateralControl());
  layout->addWidget(new LiveTuneToggle());
  QString lat_control = QString::fromStdString(Params().get("LateralControlMethod", false));
  if (lat_control == "0") {
    layout->addWidget(new PidKp());
    layout->addWidget(new PidKi());
    layout->addWidget(new PidKd());
    layout->addWidget(new PidKf());
    layout->addWidget(new IgnoreZone());
    layout->addWidget(new ShaneFeedForward());
  } else if (lat_control == "1") {
    layout->addWidget(new InnerLoopGain());
    layout->addWidget(new OuterLoopGain());
    layout->addWidget(new TimeConstant());
    layout->addWidget(new ActuatorEffectiveness());
  } else if (lat_control == "2") {
    layout->addWidget(new Scale());
    layout->addWidget(new LqrKi());
    layout->addWidget(new DcGain());
  }

  layout->addStretch(1);

  QWidget *w = new QWidget;
  w->setLayout(layout);

  return w;
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {
  // setup two main layouts
  QVBoxLayout *sidebar_layout = new QVBoxLayout();
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();
  panel_widget->setStyleSheet(R"(
    border-radius: 30px;
    background-color: #292929;
  )");

  // close button
  QPushButton *close_btn = new QPushButton("닫기");
  close_btn->setStyleSheet(R"(
    font-size: 60px;
    font-weight: bold;
    border 1px grey solid;
    border-radius: 100px;
    background-color: #292929;
  )");
  close_btn->setFixedSize(200, 200);
  sidebar_layout->addSpacing(45);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignCenter);
  QObject::connect(close_btn, SIGNAL(released()), this, SIGNAL(closeSettings()));

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, SIGNAL(reviewTrainingGuide()), this, SIGNAL(reviewTrainingGuide()));

  QPair<QString, QWidget *> panels[] = {
    {"장치", new DevicePanel(this)},
    {"네트워크", network_panel(this)},
    {"토글메뉴", toggles_panel()},
    {"정보", new DeveloperPanel()},
    {"사용자설정", user_panel(this)},
    {"튜닝", tuning_panel(this)},
  };

  sidebar_layout->addSpacing(45);
  nav_btns = new QButtonGroup();
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 65px;
        font-weight: 500;
        padding-top: 18px;
        padding-bottom: 18px;
      }
      QPushButton:checked {
        color: white;
      }
    )");

    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    panel->setContentsMargins(50, 25, 50, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::released, [=, w = panel_frame]() {
      panel_widget->setCurrentWidget(w);
    });
  }
  qobject_cast<QPushButton *>(nav_btns->buttons()[0])->setChecked(true);
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *settings_layout = new QHBoxLayout();

  sidebar_widget = new QWidget;
  sidebar_widget->setLayout(sidebar_layout);
  sidebar_widget->setFixedWidth(500);
  settings_layout->addWidget(sidebar_widget);
  settings_layout->addWidget(panel_widget);

  setLayout(settings_layout);
  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
  )");
}

void SettingsWindow::hideEvent(QHideEvent *event){
#ifdef QCOM
  HardwareEon::close_activities();
#endif
}

