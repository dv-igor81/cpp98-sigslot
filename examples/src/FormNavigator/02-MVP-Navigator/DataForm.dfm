object DataForm: TDataForm
  Left = 1232
  Top = 338
  BorderStyle = bsDialog
  Caption = 'FormNavigator '#8212' '#1056#1077#1076#1072#1082#1090#1086#1088' '#1076#1072#1085#1085#1099#1093
  ClientHeight = 302
  ClientWidth = 464
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
    Left = 170
    Top = 20
    Width = 112
    Height = 20
    Caption = #1042#1074#1086#1076' '#1076#1072#1085#1085#1099#1093
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clNavy
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object m_textLabel: TLabel
    Left = 40
    Top = 80
    Width = 33
    Height = 13
    Caption = #1058#1077#1082#1089#1090':'
  end
  object m_countLabel: TLabel
    Left = 40
    Top = 120
    Width = 43
    Height = 13
    Caption = #1057#1095#1105#1090#1095#1080#1082':'
  end
  object m_textEdit: TEdit
    Left = 120
    Top = 76
    Width = 300
    Height = 21
    TabOrder = 0
  end
  object m_countEdit: TEdit
    Left = 120
    Top = 116
    Width = 60
    Height = 21
    TabOrder = 1
    Text = '1'
  end
  object m_applyBtn: TButton
    Left = 180
    Top = 175
    Width = 130
    Height = 35
    Caption = #1055#1088#1080#1084#1077#1085#1080#1090#1100
    TabOrder = 2
    OnClick = onApplyClick
  end
  object m_goHomeBtn: TButton
    Left = 40
    Top = 255
    Width = 130
    Height = 35
    Caption = #1043#1083#1072#1074#1085#1072#1103
    TabOrder = 3
    OnClick = onGoHomeClick
  end
  object m_goResultBtn: TButton
    Left = 180
    Top = 255
    Width = 130
    Height = 35
    Caption = #1056#1077#1079#1091#1083#1100#1090#1072#1090#1099
    TabOrder = 4
    OnClick = onGoResultClick
  end
end
