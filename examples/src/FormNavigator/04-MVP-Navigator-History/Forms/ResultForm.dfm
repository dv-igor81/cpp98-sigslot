object ResultForm: TResultForm
  Left = 193
  Top = 125
  BorderStyle = bsDialog
  Caption = 'FormNavigator '#8212' '#1056#1077#1079#1091#1083#1100#1090#1072#1090#1099
  ClientHeight = 342
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
    Left = 185
    Top = 15
    Width = 102
    Height = 20
    Caption = #1056#1077#1079#1091#1083#1100#1090#1072#1090#1099
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clNavy
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object m_resultMemo: TMemo
    Left = 40
    Top = 55
    Width = 390
    Height = 230
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object m_goHomeBtn: TButton
    Left = 40
    Top = 295
    Width = 130
    Height = 35
    Caption = #1043#1083#1072#1074#1085#1072#1103
    TabOrder = 1
    OnClick = onGoHomeClick
  end
  object m_goDataBtn: TButton
    Left = 180
    Top = 295
    Width = 130
    Height = 35
    Caption = #1056#1077#1076#1072#1082#1090#1086#1088' '#1076#1072#1085#1085#1099#1093
    TabOrder = 2
    OnClick = onGoDataClick
  end
end
