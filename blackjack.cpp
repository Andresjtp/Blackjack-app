#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

class Card {
public:
    string value;
    string suit;

    Card(string v, string s) : value(v), suit(s) {}

    int getValue(int currentTotal) const {
        if (value == "J" || value == "Q" || value == "K") return 10;
        if (value == "A") return (currentTotal + 11 > 21) ? 1 : 11;
        return stoi(value);
    }

    string getDisplay() const {
        return value + " of " + suit;
    }
};

class Deck {
private:
    vector<Card> cards;

public:
    Deck() {
        reset();
    }

    void reset() {
        cards.clear();
        string suitsArray[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
        string valuesArray[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 13; ++j) {
                cards.push_back(Card(valuesArray[j], suitsArray[i]));
            }
        }
    }

    void shuffle() {
        random_shuffle(cards.begin(), cards.end());
    }

    Card drawCard() {
        if (cards.empty()) {
            cout << "Deck is empty! Reshuffling..." << endl;
            reset();
            shuffle();
        }
        Card card = cards.back();
        cards.pop_back();
        return card;
    }
};

class BlackjackGame {
private:
    vector<Card> playerHand;
    vector<Card> splitHand;
    vector<Card> dealerHand;
    Deck deck;
    int chips;
    int wins, losses, ties;
    bool isSplit;

public:
    BlackjackGame() : chips(100), wins(0), losses(0), ties(0), isSplit(false) {}

    void start() {
        char playAgain = 'Y';
        cout << "\U0001F3B4 Welcome to Blackjack! You start with 100 chips." << endl;

        while (playAgain == 'Y' || playAgain == 'y') {
            deck.reset();
            deck.shuffle();
            playerHand.clear();
            splitHand.clear();
            dealerHand.clear();
            isSplit = false;

            int bet;
            cout << "\n\U0001F4B0 You have " << chips << " chips. Enter your bet: ";
            cin >> bet;
            if (bet > chips || bet <= 0) {
                cout << "❌ Invalid bet. Try again.\n";
                continue;
            }

            dealInitialCards();

            
            if (calculateTotal(playerHand) == 21) {
                printHand(playerHand, "Player");
                printHand(dealerHand, "Dealer", false);
                cout << "\U0001F389 Blackjack! Automatic win! +" << bet << " chips." << endl;
                chips += bet;
                wins++;
                printStats();
                goto end_round;
            }

            if (playerTurn(bet)) {
                revealDealerCard();
                dealerTurn();
                if (isSplit) {
                    resolveSplitHands(playerHand, bet);
                    resolveSplitHands(splitHand, bet);
                } else {
                    resolveGame(bet);
                }
            } else {
                revealDealerCard();
                chips -= bet;
                losses++;
            }

            printStats();
end_round:
            if (chips <= 0) {
                cout << "\U0001F4A5 You're out of chips! Game over." << endl;
                break;
            }

            cout << "\nPlay again? (Y/N): ";
            cin >> playAgain;
        }

        cout << "\nThanks for playing! Final Stats:\n";
        printStats();
    }

private:
    void dealInitialCards() {
        playerHand.push_back(deck.drawCard());
        playerHand.push_back(deck.drawCard());
        dealerHand.push_back(deck.drawCard());
        dealerHand.push_back(deck.drawCard());
    }

    int calculateTotal(const vector<Card>& hand) {
        int total = 0;
        int aceCount = 0;
        for (size_t i = 0; i < hand.size(); ++i) {
            if (hand[i].value == "A") aceCount++;
            total += hand[i].getValue(total);
        }
        while (total > 21 && aceCount > 0) {
            total -= 10;
            aceCount--;
        }
        return total;
    }

    void printHand(const vector<Card>& hand, string owner, bool showAll = true) {
        cout << owner << "'s hand: ";
        for (size_t i = 0; i < hand.size(); ++i) {
            if (!showAll && owner == "Dealer" && i == 1) {
                cout << "[Hidden] ";
            } else {
                cout << hand[i].getDisplay() << " ";
            }
        }
        if (showAll || owner == "Player") {
            cout << "(Total: " << calculateTotal(hand) << ")";
        }
        cout << endl;
    }

    bool playSingleHand(vector<Card>& hand) {
        char choice;
        while (true) {
            printHand(hand, "Player");
            printHand(dealerHand, "Dealer", false);
            cout << "Do you want to (H)it or (S)tand? ";
            cin >> choice;

            if (choice == 'H' || choice == 'h') {
                hand.push_back(deck.drawCard());
                if (calculateTotal(hand) > 21) {
                    printHand(hand, "Player");
                    cout << "\U0001F4A5 Bust!" << endl;
                    return false;
                }
            } else if (choice == 'S' || choice == 's') {
                return true;
            }
        }
    }

    bool playerTurn(int bet) {
        if (playerHand.size() == 2 && playerHand[0].value == playerHand[1].value && chips >= bet * 2) {
            char splitChoice;
            cout << "\U0001F0CF You have a pair. Do you want to split? (Y/N): ";
            cin >> splitChoice;
            if (splitChoice == 'Y' || splitChoice == 'y') {
                isSplit = true;
                splitHand.push_back(playerHand[1]);
                playerHand.pop_back();
                playerHand.push_back(deck.drawCard());
                splitHand.push_back(deck.drawCard());
                cout << "\n\U0001F449 Playing first hand:" << endl;
                playSingleHand(playerHand);
                cout << "\n\U0001F449 Playing second hand:" << endl;
                playSingleHand(splitHand);
                return true;
            }
        }
        return playSingleHand(playerHand);
    }

    void revealDealerCard() {
        cout << "\n\U0001F50D Revealing dealer's hand..." << endl;
        printHand(dealerHand, "Dealer");
    }

    void dealerTurn() {
        while (calculateTotal(dealerHand) < 17) {
            dealerHand.push_back(deck.drawCard());
        }
        printHand(dealerHand, "Dealer");
    }

    void resolveGame(int bet) {
        int playerTotal = calculateTotal(playerHand);
        int dealerTotal = calculateTotal(dealerHand);
        if (dealerTotal > 21 || playerTotal > dealerTotal) {
            cout << "✅ You win! +" << bet << " chips." << endl;
            chips += bet;
            wins++;
        } else if (dealerTotal > playerTotal) {
            cout << "❌ Dealer wins. -" << bet << " chips." << endl;
            chips -= bet;
            losses++;
        } else {
            cout << "\U0001F91D It's a tie!" << endl;
            ties++;
        }
    }

    void resolveSplitHands(vector<Card>& hand, int bet) {
        int total = calculateTotal(hand);
        int dealerTotal = calculateTotal(dealerHand);
        if (total > 21) {
            chips -= bet;
            losses++;
            cout << "\U0001F534 Hand busts. -" << bet << " chips." << endl;
        } else if (dealerTotal > 21 || total > dealerTotal) {
            chips += bet;
            wins++;
            cout << "✅ Hand wins! +" << bet << " chips." << endl;
        } else if (dealerTotal > total) {
            chips -= bet;
            losses++;
            cout << "❌ Dealer wins this hand. -" << bet << " chips." << endl;
        } else {
            ties++;
            cout << "\U0001F91D This hand is a tie." << endl;
        }
    }

    void printStats() {
        cout << "\n\U0001F4CA Wins: " << wins << " | Losses: " << losses << " | Ties: " << ties << " | Chips: " << chips << endl;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0)));
    BlackjackGame game;
    game.start();
    return 0;
}
