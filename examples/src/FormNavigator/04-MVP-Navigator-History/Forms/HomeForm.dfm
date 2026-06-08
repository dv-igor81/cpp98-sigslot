object HomeForm: THomeForm
  Left = 193
  Top = 125
  BorderStyle = bsDialog
  Caption = 'FormNavigator '#8212' '#1043#1083#1072#1074#1085#1072#1103
  ClientHeight = 302
  ClientWidth = 470
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object m_titleLabel: TLabel
    Left = 155
    Top = 20
    Width = 122
    Height = 20
    Caption = #1043#1083#1072#1074#1085#1086#1077' '#1084#1077#1085#1102
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clNavy
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object m_infoLabel: TLabel
    Left = 24
    Top = 46
    Width = 390
    Height = 107
    AutoSize = False
    Caption = #1044#1072#1085#1085#1099#1077' '#1077#1097#1105' '#1085#1077' '#1079#1072#1076#1072#1085#1099'. '#1053#1072#1078#1084#1080#1090#1077' '#171#1056#1077#1076#1072#1082#1090#1086#1088' '#1076#1072#1085#1085#1099#1093#187'.'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object m_goDataBtn: TButton
    Left = 40
    Top = 240
    Width = 130
    Height = 35
    Caption = #1056#1077#1076#1072#1082#1090#1086#1088' '#1076#1072#1085#1085#1099#1093
    TabOrder = 0
    OnClick = onGoDataClick
  end
  object m_goResultBtn: TButton
    Left = 180
    Top = 240
    Width = 130
    Height = 35
    Caption = #1056#1077#1079#1091#1083#1100#1090#1072#1090#1099
    TabOrder = 1
    OnClick = onGoResultClick
  end
  object m_exitBtn: TButton
    Left = 320
    Top = 240
    Width = 130
    Height = 35
    Caption = #1042#1099#1093#1086#1076
    TabOrder = 2
    OnClick = onExitClick
  end
end
