int redLed = 8;
int greenLed = 7;

String listWithCards[10];
int amountOfCards = 0;

void RemoteOpenDoor() {
  digitalWrite(greenLed, HIGH);
  delay(3000);
  digitalWrite(greenLed, LOW);
}
void ClearCardsFromList(){
    for(int i = 0; i < amountOfCards; i++){
      listWithCards[i] = "";
      }  
    amountOfCards = 0;
}
void GetCardsFromPc(String input) {
    listWithCards[amountOfCards] = input;
    amountOfCards++;
}

void ScanCard(String input){
  for(int i = 0; i < amountOfCards; i++){
    if(listWithCards[i] == input){   
      digitalWrite(greenLed, HIGH);
      delay(3000);
      digitalWrite(greenLed, LOW);
      return;
    }
  }
  digitalWrite(redLed, HIGH);
  delay(3000);
  digitalWrite(redLed, LOW);
}

void setup() {
  Serial.begin(9600);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
}

void loop() {
  if(amountOfCards == 10){
    ClearCardsFromList();
  }
  if (Serial.available()) {
    String input = Serial.readStringUntil('>');
    
    if (input.equals("<REMOTEOPENDOOR")) {
      RemoteOpenDoor();
    }
    else if (input.startsWith("<ADDCARD")){
      GetCardsFromPc(input.substring(8));
    }
    else if (input.startsWith("<SCANCARD")){
      ScanCard(input.substring(9));
    }
    else if (input.equals("<CLEARALLCARDS")){
      ClearCardsFromList();
    }
  }
}
