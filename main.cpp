#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <stack>
#include <stdatomic.h>
#include <string>
#include <type_traits>
#include <vector>

using namespace std;

const int NUM_DECKS = 6;
const int STARTING_MONEY = 1000;
const vector<string> VALID_MOVES = {"hit", "stand", "split", "double",
                                    "h",   "st",    "sp",    "d"};
bool isValidMove(std::string &text);
void convertToLower(string &s) {
  for (auto &c : s)
    c = std::tolower((unsigned char)c);
}

int getRank(int card) { return card % 13; }

enum Rank {
  One,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Jack,
  Queen,
  King,
  Ace
};

enum Suit { Clubs, Spades, Hearts, Diamonds };

enum Move { Hit, Stand, Double, Split };

std::ostream &operator<<(std::ostream &os, Suit s) {
  static const char *names[] = {"Clubs", "Diamonds", "Hearts", "Spades"};
  return os << names[static_cast<int>(s)];
}

std::ostream &operator<<(std::ostream &os, Rank r) {
  static const char *names[] = {"2", "3",  "4",    "5",     "6",    "7",  "8",
                                "9", "10", "Jack", "Queen", "King", "Ace"};
  return os << names[static_cast<int>(r)];
}

struct Card {
  Rank rank;
  Suit suit;

  Card(int num) {
    rank = static_cast<Rank>(num % 13);
    suit = static_cast<Suit>(num % 4);
  }

  Card(const Card &other) : rank(other.rank), suit(other.suit) {}

  Card() : rank(Ace), suit(Hearts) {}
  void printCard() { cout << rank << " of " << suit; }

  bool isFaceCard() {
    return rank == Ten || rank == Jack || rank == Queen || rank == King;
  }
};

class Shoot {
public:
  vector<int> cards;

  Shoot(int numDecks) {
    for (int i = 0; i < numDecks; i++) {
      for (int j = 0; j < 52; j++) {
        cards.push_back(j);
      }
    }
  }

  Card deal() {
    int cards_left = cards.size();
    if (cards_left == 0) {
      return Card(0);
    }
    int random_idx = rand() % cards_left;
    int cardAsInt = cards[random_idx];
    Card card = Card(cardAsInt);
    cards.erase(cards.begin() + random_idx);
    return card;
  }
};

class Hand {
public:
  int sum;
  vector<Card> cards;
  bool done = false;
  int bet = 0;
  Hand() {
    sum = 0;
    cards = {};
  }

  Hand(const vector<Card> &cardsIn) : sum(0), cards(cardsIn) { sumUpHand(); }

  Hand(vector<Card> &cards) {
    for (auto c : cards) {
      this->cards.push_back(c);
    }
    sumUpHand();
  }

  void addCard(Card card) {
    cards.push_back(card);
    sumUpHand();
  }

  void sumUpHand() {
    sum = 0;
    for (auto c : cards) {
      if (c.isFaceCard()) {
        sum += 10;
      } else if (c.rank == Ace) {
        sum += 11;
      } else {
        sum += static_cast<int>(c.rank) + 2;
      }
    }
  }

  bool isA_BlackJack() {
    return cards.size() == 2 &&
               (cards[0].rank == Ace && cards[1].isFaceCard()) ||
           (cards[1].rank == Ace && cards[1].isFaceCard());
  }

  void printHand() {
    for (auto c : cards) {
      c.printCard();
    }
  }
};

class Player {
public:
  vector<Hand> hands;
  int money;

  void initialDeal(Shoot &shoot) {
    vector<Card> cards;
    cards.push_back(shoot.deal());
    cards.push_back(shoot.deal());
    hands.push_back(Hand(cards));
  }

  void printHand(int idx) {
    int i = 0;
    for (Card c : hands[idx].cards) {
      c.printCard();
      if (i == 0) {
        cout << " and ";
      }
      i++;
    }
    cout << endl;
  }

  bool allHandsDone() {
    for (auto h : hands) {
      if (!h.done) {
        return false;
      }
    }
    return true;
  }

  Player(int dollars) { money = dollars; }

  Move promptUserForValidMove(Hand &hand) {
    string text = "";
    cout << "Your total is " << hand.sum << endl;

    cout << "Enter a move (Hit, Stand";
    if (money >= hand.bet) {
      cout << ", Double";
    }
    if (hand.cards.size() == 2 && hand.cards[0].rank == hand.cards[1].rank) {
      cout << ", Split";
    }
    cout << "):";
    cin >> text;

    if (!isValidMove(text)) {
      return promptUserForValidMove(hand);
    }
    if (text == "double" || text == "d") {
      if (money < hand.bet) {
        cout << "Not enough money to double." << endl;
        return promptUserForValidMove(hand);
      }
      return Double;
    }
    if (text == "hit" || text == "h") {
      return Hit;
    }
    if (text == "split" || text == "sp") {
      if (hand.cards.size() != 2 || hand.cards[0].rank != hand.cards[1].rank) {
        cout << "Cannot split on this hand" << endl;
        return promptUserForValidMove(hand);
      }

      return Split;
    }
    return Stand;
  }

  void handleSplit(Hand &hand, Shoot &shoot) {
    if (hand.cards[0].rank != hand.cards[1].rank || hand.cards.size() != 2) {
      cout << "error: tried to split a noneligible hand" << endl;
      return;
    }
    hands.push_back(Hand({hand.cards[1]}));
    hand.cards[1] = shoot.deal();
    cout << "new left hand is: ";
    hand.printHand();

    hands[hands.size() - 1].addCard(shoot.deal());
  }

  bool allHandsBusted() {
    for (Hand h : hands) {
      if (h.sum <= 21) {
        return false;
      }
    }
    return true;
  }

  int strength() { return hands[0].sum; }

  void muck() { hands = {}; }
};

class Dealer {
public:
  Hand hand;
  Dealer() { hand = {}; }

  void initialDeal(Shoot &shoot) {
    vector<Card> cards;
    cards.push_back(shoot.deal());
    cards.push_back(shoot.deal());
    hand = Hand(cards);
  }

  bool printFirstCard() {
    hand.cards[0].printCard();
    if (hand.sum == 21) {
      cout << "dealer has blackjack" << endl;
      return 1;
    }
    cout << endl;
    return 0;
  }

  void addCardToHand(Card card) {
    hand.addCard(card);
    hand.sumUpHand();
  }
};

bool isValidMove(string &text) {
  convertToLower(text);
  return find(VALID_MOVES.begin(), VALID_MOVES.end(), text) !=
         VALID_MOVES.end();
}

void processMove(Player &player, Move move, Hand *hand, Shoot &shoot) {
  if (move == Hit) {
    cout << "player hits, the ";
    Card card = shoot.deal();
    card.printCard();
    cout << " is dealt. " << endl;
    hand->addCard(card);
    if (hand->sum >= 21) {
      hand->done = true;
      cout << "Player busts!" << endl;
    }
    return;
  }
  if (move == Stand) {
    cout << "Player stands" << endl;
    hand->done = true;
    return;
  }
  if (move == Double) {
    cout << "Player doubles for another $" << hand->bet << endl;
    Card card = shoot.deal();
    card.printCard();
    cout << " is dealt" << endl;
    hand->addCard(card);
    hand->sumUpHand();
    hand->bet = hand->bet << 1;
    if (hand->sum > 21) {
      cout << "Player busted! (total: " << hand->sum << ")\n";
    }
    hand->done = true;
    return;
  }
  if (move == Split) {
    player.handleSplit(*hand, shoot);
  }
  // TODO: add split logic
}

void playDealer(Dealer &dealer, Shoot &shoot) {
  cout << "Dealer turns over ";
  dealer.hand.cards[1].printCard();
  while (dealer.hand.sum < 17) {
    Card card = shoot.deal();
    cout << "\nDealer hits for a ";
    card.printCard();
    cout << endl;
    dealer.addCardToHand(card);
  }
  if (dealer.hand.sum > 21) {
    cout << "Dealer busted!\n";
  }
  cout << endl;
}

int returnGainFromHand(Hand &playerHand, Hand &dealerHand) {
  if (playerHand.sum > dealerHand.sum && playerHand.sum <= 21) {
    return playerHand.bet;
  }
  if (dealerHand.sum > 21) {
    return playerHand.bet;
  }

  if (playerHand.sum == dealerHand.sum) {
    return 0;
  }
  return -1 * playerHand.bet;
}

void evaluateHands(Player &player, Dealer &dealer) {
  for (auto hand : player.hands) {
    int gain = returnGainFromHand(hand, dealer.hand);
    if (gain == 0) {
      cout << "It's a push" << endl;
    } else if (gain > 1) {
      cout << "Player wins $" << gain << endl;
    } else {
      cout << "Player loses $" << -1 * gain << endl;
    }
    player.money += gain;
  }
}

void playHand(Player &player, Dealer &dealer, Shoot &shoot) {
  if (player.money <= 0) {
    cout << "All out of money" << endl;
    return;
  }
  cout << "Current stack: $" << player.money << endl;
  cout << "Enter Bet size: ";
  int bet = 0;
  cin >> bet;
  while (int(bet) > player.money) {
    cout << "\nNot enough money for that bet" << endl;
    cin >> bet;
    cout << "Enter Bet size: ";
  }
  cout << "Player bet $" << bet << endl;
  player.initialDeal(shoot);
  player.hands[0].bet = bet;
  cout << "Player has ";
  player.printHand(0);
  if (player.hands[0].isA_BlackJack()) {
    cout << "Blackjack!" << endl;
    player.money += int(bet * 1.5);
    cout << "Player wins " << int(bet * 1.5) << endl;
  }
  dealer.initialDeal(shoot);

  cout << "Dealer has ";
  dealer.printFirstCard();
  Hand *hand = &player.hands[0];

  string text = "";
  while (!player.allHandsDone()) {
    Move move = player.promptUserForValidMove(*hand);
    processMove(player, move, &player.hands[0], shoot);
  }
  if (!player.allHandsBusted()) {
    playDealer(dealer, shoot);
  }
  evaluateHands(player, dealer);

  player.muck();
}

int main() {
  srand(time(0));
  Player player(STARTING_MONEY);
  Shoot shoot(NUM_DECKS);
  Dealer dealer;
  while (player.money > 0) {
    playHand(player, dealer, shoot);
  }
  cout << "All out of money" << endl;
  return 0;
}
