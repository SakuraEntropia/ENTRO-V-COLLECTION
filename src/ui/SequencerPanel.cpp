#include "ui/SequencerPanel.h"
namespace triad::ui {
SequencerPanel::SequencerPanel(EditorHost& h,std::function<void()> close):host_(h) {
    auto& seq=host_.sequencer();
    close_.onClick=std::move(close);play_.onClick=[this]{startPattern();};
    clear_.onClick=[this]{host_.sequencer().clearPattern(host_.sequencer().selectedPattern());refresh();};
    for(auto* button:{&close_,&play_,&clear_})addAndMakeVisible(*button);
    for(int p=0;p<seq.patternCount();++p)bank_.addItem("BANK "+juce::String(p+1),p+1);
    bank_.onChange=[this]{host_.sequencer().selectPattern(bank_.getSelectedId()-1);refresh();};
    for(int p=0;p<(seq.maximumSteps()+15)/16;++p)page_.addItem(juce::String(p*16+1)+" - "+juce::String((p+1)*16),p+1);
    page_.onChange=[this]{pageIndex_=page_.getSelectedId()-1;refresh();};page_.setSelectedId(1,juce::dontSendNotification);
    addAndMakeVisible(bank_);addAndMakeVisible(page_);
    length_.setSliderStyle(juce::Slider::IncDecButtons);length_.setTextBoxStyle(juce::Slider::TextBoxLeft,false,55,22);
    length_.setRange(1,seq.maximumSteps(),1);length_.setTextValueSuffix(" steps");
    length_.onValueChange=[this]{if(!refreshing_)host_.sequencer().setStepCount((int)length_.getValue());};addAndMakeVisible(length_);
    gate_.setSliderStyle(juce::Slider::LinearHorizontal);gate_.setTextBoxStyle(juce::Slider::TextBoxRight,false,45,22);
    gate_.setRange(.05,1,.01);gate_.setTooltip("Gate fraction; TIE holds a chord into the next step.");
    gate_.onValueChange=[this]{if(!refreshing_)host_.sequencer().setGate((float)gate_.getValue());};addAndMakeVisible(gate_);
    for(int c=0;c<16;++c){
        enabled_[c].onClick=[this,c]{host_.sequencer().setStepActive(pageIndex_*16+c,enabled_[c].getToggleState());};addAndMakeVisible(enabled_[c]);
        tie_[c].onClick=[this,c]{host_.sequencer().setPatternTie(host_.sequencer().selectedPattern(),pageIndex_*16+c,tie_[c].getToggleState());};addAndMakeVisible(tie_[c]);
        for(int n=0;n<seq.polyphony();++n){auto& l=notes_[c][n];l.setEditable(true);l.setJustificationType(juce::Justification::centred);
            l.setColour(juce::Label::backgroundColourId,juce::Colour(0xff25262a));
            l.onTextChange=[this,c,n]{if(refreshing_)return;auto& s=host_.sequencer();auto text=notes_[c][n].getText();
                s.setPatternNote(s.selectedPattern(),pageIndex_*16+c,n,text=="-"?-1:text.getIntValue());refresh();};addAndMakeVisible(l);}
        velocity_[c].setEditable(true);velocity_[c].setJustificationType(juce::Justification::centred);
        velocity_[c].onTextChange=[this,c]{if(!refreshing_)host_.sequencer().setStepVelocity(pageIndex_*16+c,velocity_[c].getText().getIntValue());};addAndMakeVisible(velocity_[c]);
    }
    refresh();startTimerHz(12);
}
void SequencerPanel::startPattern(){
    const auto& d=host_.definition();
    auto set=[this](const juce::String& id,float v){if(auto* p=host_.apvts().getParameter(id))p->setValueNotifyingHost(p->convertTo0to1(v));};
    const auto prefix=d.id=="matriarch"?juce::String("mat"):d.id=="polybrute"?juce::String("poly"):juce::String("one.p1");
    auto* enabled=host_.apvts().getRawParameterValue(prefix+".arp.enabled");
    const bool stop=enabled!=nullptr && enabled->load()>.5f;
    if(stop){set(prefix+".arp.enabled",0);return;}
    set(prefix+".arp.mode",5);set(prefix+".arp.enabled",1);
}
void SequencerPanel::refresh(){
    refreshing_=true;auto& s=host_.sequencer();const int bank=s.selectedPattern();
    bank_.setSelectedId(bank+1,juce::dontSendNotification);length_.setValue(s.stepCount(),juce::dontSendNotification);gate_.setValue(s.gate(),juce::dontSendNotification);
    for(int c=0;c<16;++c){const int step=pageIndex_*16+c;enabled_[c].setButtonText(juce::String(step+1));enabled_[c].setToggleState(s.isStepActive(step),juce::dontSendNotification);
        tie_[c].setButtonText("TIE");tie_[c].setToggleState(s.patternTie(bank,step),juce::dontSendNotification);
        for(int n=0;n<s.polyphony();++n){const int note=s.patternNote(bank,step,n);notes_[c][n].setText(note<0?"-":juce::String(note),juce::dontSendNotification);
            notes_[c][n].setTooltip(note<0?"Empty note lane (-1); double click to edit MIDI note number":juce::MidiMessage::getMidiNoteName(note,true,true,3));}
        velocity_[c].setText(juce::String(s.stepVelocity(step)),juce::dontSendNotification);
    }refreshing_=false;repaint();
}
void SequencerPanel::timerCallback(){if(host_.sequencer().currentStep()!=lastStep_){lastStep_=host_.sequencer().currentStep();repaint();}}
void SequencerPanel::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff111216));g.setColour(juce::Colour(0xffc4c8cb));g.setFont(juce::Font(juce::FontOptions(15.0f,juce::Font::bold)));
    g.drawText("PATTERN EDITOR  /  "+host_.instrumentName(),20,10,600,25,juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(11.0f)));g.drawText("MIDI notes 0-127, '-' = empty. Double click numbers to edit. Play from C4 (60) for original pitch.",20,getHeight()-29,getWidth()-40,22,juce::Justification::centredLeft);
    if(lastStep_>=pageIndex_*16 && lastStep_<(pageIndex_+1)*16){g.setColour(juce::Colour(0xffd2803f));float w=(getWidth()-96)/16.0f;g.drawRect(80+(int)(w*(lastStep_%16)),88,(int)w,getHeight()-126,2);}
    for(int n=0;n<host_.sequencer().polyphony();++n)g.drawText("NOTE "+juce::String(n+1),8,126+n*27,68,24,juce::Justification::centredRight);
    g.drawText("VELOCITY",8,126+host_.sequencer().polyphony()*27,68,24,juce::Justification::centredRight);
}
void SequencerPanel::resized(){
    close_.setBounds(getWidth()-92,12,74,26);bank_.setBounds(20,48,120,28);page_.setBounds(150,48,120,28);length_.setBounds(280,48,130,28);gate_.setBounds(425,48,145,28);play_.setBounds(585,48,78,28);clear_.setBounds(678,48,112,28);
    const int w=(getWidth()-96)/16;
    for(int c=0;c<16;++c){int x=80+c*w;enabled_[c].setBounds(x,94,w,26);
        for(int n=0;n<host_.sequencer().polyphony();++n)notes_[c][n].setBounds(x+3,126+n*27,w-6,24);
        int bottom=126+host_.sequencer().polyphony()*27;velocity_[c].setBounds(x+3,bottom,w-6,24);tie_[c].setBounds(x,bottom+28,w,24);}
}
}
