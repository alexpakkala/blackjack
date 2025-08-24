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

enum Move { Hit, Stand, Double };

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

  bool isFaceCard() { return rank == Jack || rank == Queen || rank == King; }
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
  Hand() {
    sum = 0;
    cards = {};
  }

  Hand(vector<Card> &cards) {
    for (auto c : cards) {
      this->cards.push_back(c);
    }
    sumUpHand();
  }

  void addCard(Card card) { cards.push_back(card); }

  void sumUpHand() {
    sum = 0;
    for (auto c : cards) {
      this->cards.push_back(c);
      if (c.isFaceCard()) {
        sum += 10;
      } else if (c.rank == Ace) {
        sum += 11;
      } else {
        sum += static_cast<int>(c.rank) + 2;
      }
    }
  }
};

class Player {
public:
  vector<Hand> hands;
  int money;
  void initialDeal(Shoot &shoot) {
    vector<Card> cards;
    cards.resize(2);
    cards[0] = shoot.deal();
    cards[1] = shoot.deal();
    hands.push_back(cards);
  }

  void printHand(int idx) {
    int i = 0;
    for (Card c : hands[idx].cards) {
      if (i > 0) {
        cout << " and ";
      }
      c.printCard();

      i++;
    }
    cout << endl;
  }

  Player(int dollars) { money = dollars; }

  int strength() { return hands[0].sum; }

  void muck() { hands.resize(0); }
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
};

bool isValidMove(string &text) {
  convertToLower(text);
  return find(VALID_MOVES.begin(), VALID_MOVES.end(), text) !=
         VALID_MOVES.end();
}

void processMove(Player &player, string &text, Hand *hand, Shoot &shoot,
                 int bet) {
  if (text == "hit" || text == "h") {
    cout << "player hits, the ";
    Card card = shoot.deal();
    card.printCard();
    cout << " is dealt. " << endl;
    hand->addCard(card);
    hand->sumUpHand();
    if (hand->sum >= 21) {
      hand->done = true;
    }
    return;
  }
  if (text == "stand" || text == "st") {
    cout << "Player stands" << endl;
    hand->done = true;
    return;
  }
  if (text == "double" || text == "d") {
    cout << "Player doubles for another $" << bet << endl;
    Card card = shoot.deal();
    card.printCard();
    hand->addCard(card);
    hand->sumUpHand();
    if (hand->sum >= 21) {
      hand->done = true;
    }
    return;
  }
  // TODO: add split logic
}

void playHand(Player &player, Dealer &dealer, Shoot &shoot) {
  cout << "Current stack: $" << player.money << endl;
  cout << "Enter Bet size: ";
  int bet = 0;
  cin >> bet;
  cout << "Player bet $" << bet << endl;

  player.initialDeal(shoot);
  cout << "Player has ";
  player.printHand(0);

  dealer.initialDeal(shoot);
  cout << "Dealer has ";
  dealer.printFirstCard();

  while (true) {
    string text = "";
    cout << "Enter move (Hit, Stand, Double, Split)";
    cin >> text;
    while (!isValidMove(text)) {
      cout << "Enter a valid move (Hit, Stand, Double)";
      cin >> text;
      processMove(player, text, &player.hands[0], shoot, bet);
    }
    break;
  }

  player.muck();
}

int main() {
  srand(time(0));
  Player player(STARTING_MONEY);
  Shoot shoot(NUM_DECKS);
  Dealer dealer;
  while (true) {
    playHand(player, dealer, shoot);
  }
  return 0;
}
