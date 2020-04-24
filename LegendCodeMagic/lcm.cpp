#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdbool>

using namespace std;

enum		ActionType
{
	Pass,
	Summon,
	Attack,
	Use,
	Pick,
};

enum		Location
{
	Enemy = -1,
	Hand = 0,
	Mine = 1
};

enum		CardType
{
	Monster,
	Green,
	Red,
	Blue
};

struct		Player
{
	int     hp;
	int     mana;
	int     deck;
	int     rune;
	int     draw;
};

struct		Card
{
	int				number;
	int				id;
	Location		location;
	CardType		type;
	int				cost;
	int				atk;
	int				def;
	int				hpChange;
	int				hpChangeEnemy;
	int				draw;
	bool			breakthrough = false;
	bool			charge = false;
	bool			drain = false;
	bool			guard = false;
	bool			lethal = false;
	bool			ward = false;
	Card			*target = nullptr;

	double			ratio();

	static void		swap(Card &A, Card &B);
	static void		sortByCostAndRatio(vector<Card> &cards);
	static void		sortByDefByAtlkByLocation(vector<Card> &cards);
};

double		Card::ratio()
{
	double ratio;
	double divisor = cost == 0 ? 1 : cost;

	if (cost == 1)
		divisor = 1 * 1.33;
	if (type == CardType::Monster || type == CardType::Green)
		ratio = double(atk + def + breakthrough + charge + guard + drain + lethal + ward +	hpChange - hpChangeEnemy + draw) / divisor;
	else
		ratio = double(-atk - def + breakthrough + charge + guard + drain + lethal + ward + hpChange - hpChangeEnemy + draw) / divisor;
	return (ratio);
}

void		Card::swap(Card &A, Card &B)
{
	Card tmp;

	tmp = A;
	A = B;
	B = tmp;
}

void		Card::sortByCostAndRatio(vector<Card> &cards)
{
	vector<Card>::iterator it;

	for (it = cards.begin(); it < cards.end() - 1; it++)
	{
		vector<Card>::iterator next = it + 1;
		if ((*it).cost > (*next).cost)
		{
			Card::swap(*it, *next);
			it = cards.begin();
		}
	}

	for (it = cards.begin(); it < cards.end() - 1; it++)
	{
		vector<Card>::iterator next = it + 1;
		if ((*it).ratio() > (*next).ratio() and (*it).cost == (*next).cost)
		{
			Card::swap(*it, *next);
			it = cards.begin();
		}
	}
}

void		Card::sortByDefByAtlkByLocation(vector<Card> &cards)
{
	vector<Card>::iterator it;

	for (it = cards.begin(); it < cards.end() - 1; it++)
	{
		vector<Card>::iterator next = it + 1;
		if ((*it).location < (*next).location)
		{
			Card::swap((*it), (*next));
			it = cards.begin();
		}
	}

	//Sort Opponent Cards
	{
		for (it = cards.begin(); it < cards.end() - 1; it++)
		{
			vector<Card>::iterator next = it + 1;
			if ((*it).guard < (*next).guard && (*it).location == (*next).location && (*it).location == Location::Enemy)
			{
				Card::swap((*it), (*next));
				it = cards.begin();
			}
		}

		for (it = cards.begin(); it < cards.end() - 1; it++)
		{
			vector<Card>::iterator next = it + 1;
			if ((*it).def < (*next).def && (*it).location == (*next).location && (*it).location == Location::Enemy && (*it).guard == (*next).guard)
			{
				Card::swap((*it), (*next));
				it = cards.begin();
			}
		}
		for (it = cards.begin(); it < cards.end() - 1; it++)
		{
			vector<Card>::iterator next = it + 1;
			if ((*it).atk < (*next).atk && (*it).location == (*next).location && (*it).location == Location::Enemy && (*it).guard == (*next).guard && (*it).def > (*next).def)
			{
				Card::swap((*it), (*next));
				it = cards.begin();
			}
		}
	}

	//Sort My Cards
	{
		for (it = cards.begin(); it < cards.end() - 1; it++)
		{
			vector<Card>::iterator next = it + 1;
			if ((*it).atk < (*next).atk && (*it).location == (*next).location && (*it).location == Location::Mine)
			{
				Card::swap((*it), (*next));
				it = cards.begin();
			}
		}
		for (it = cards.begin(); it < cards.end() - 1; it++)
		{
			vector<Card>::iterator next = it + 1;
			if ((*it).def < (*next).def && (*it).location == (*next).location && (*it).location == Location::Mine && (*it).atk == (*next).atk)
			{
				Card::swap((*it), (*next));
				it = cards.begin();
			}
		}
	}
}

struct		State
{
	int				opponentHand;
	int				opponentActions;
	Player			player;
	Player			enemy;
	vector<Card>	cards;
};

struct		Action
{
	ActionType	type = ActionType::Pass;
	int			id = -1;
	int			target = -1;

	void	summon(int _id);
	void	pick(int _id);
	void	use(int _id);
	void	use(int _id, int _target);
	void	attack(int _id);
	void	attack(int _id, int _target);

	void	print(ostream &flux);
};

void		Action::summon(int _id)
{
	type = ActionType::Summon;
	id = _id;
}

void		Action::pick(int _id)
{
	type = ActionType::Pick;
	id = _id;
}

void		Action::use(int _id)
{
	type = ActionType::Use;
	id = _id;
}

void		Action::use(int _id, int _target)
{
	type = ActionType::Use;
	id = _id;
	target = _target;
}

void		Action::attack(int _id)
{
	type = ActionType::Attack;
	id = _id;
}

void		Action::attack(int _id, int _target)
{
	type = ActionType::Attack;
	id = _id;
	target = _target;
}

void		Action::print(ostream &flux)
{
	if (type == ActionType::Pass)
		return;
	else if (type == ActionType::Summon)
		flux << "SUMMON " << id << ";";
	else if (type == ActionType::Attack)
		flux << "ATTACK " << id << " " << target << ";";
	else if (type == ActionType::Use)
		flux << "USE " << id << " " << target << ";";
	else if (type == ActionType::Pick)
		flux << "PICK " << id;
}

void		chooseTarget(vector<Card> &cards)
{
	Card::sortByDefByAtlkByLocation(cards);
	for (Card &card : cards)
	{
		cerr << card.location << " " << card.id << " atk: " << card.atk << " def: " << card.def << endl;
	}
	for (Card &target : cards)
	{
		if (target.location != Location::Enemy) continue;
		if (!target.guard) continue;
		if (target.def <= 0) continue;
		
		for (Card &card  : cards)
		{
			if (card.location != Location::Mine) continue;
			if (card.atk != target.def) continue;
			if (card.target) continue;

			card.target = &target;
			if (target.ward)
				target.ward = false;
			else
				target.def -= card.atk;
			break;
		}

		if (target.def <= 0)
			continue;

		for (Card &card : cards)
		{
			if (card.location != Location::Mine) continue;
			if (card.atk <= target.def) continue;
			if (card.target) continue;

			card.target = &target;
			if (target.ward)
				target.ward = false;
			else
				target.def -= card.atk;
			break;
		}

		if (target.def <= 0)
			continue;

		for (Card &card : cards)
		{
			if (card.location != Location::Mine) continue;
			if (card.atk <= 0) continue;
			if (card.target) continue;

			card.target = &target;
			if (target.ward)
				target.ward = false;
			else
				target.def -= card.atk;
		}
	}
}

struct		Agent
{
	State			state;
	vector<Action>	turn;

	void	attack(vector<Card> cards);

	void	read(void);
	void	think(void);
	void	play(void);
};

void		Agent::read(void)
{
	//Information Player
	for (int i = 0; i < 2; i++)
	{
		Player *player;

		if (i == 0)
			player = &state.player;
		else
			player = &state.enemy;

		cin >> player->hp >> player->mana >> player->deck >> player->rune >> player->draw; cin.ignore();
	}
	
	//Information Last Turn Enemy
	cin >> state.opponentHand >> state.opponentActions; cin.ignore();

	//Unused Parameters of Enemy
	for (int i = 0; i < state.opponentActions; i++)
	{
		string cardNumberAndAction;
		getline(cin, cardNumberAndAction);
	}
	
	//Information Cards
	{
		int cardCount;
		cin >> cardCount; cin.ignore();

		state.cards.clear();
		for (int i = 0; i < cardCount; i++)
		{
			state.cards.emplace_back();
			Card &card = state.cards.back();

			int		location;
			int		type;
			string	abilities;

			cin >> card.number >> card.id >> location >> type >> card.cost >> card.atk >> card.def >> abilities >> card.hpChange >> card.hpChangeEnemy >> card.draw; cin.ignore();
			card.location = (Location)location;
			card.type = (CardType)type;
			if (abilities[0] != '-') card.breakthrough = true;
			if (abilities[1] != '-') card.charge = true;
			if (abilities[2] != '-') card.drain= true;
			if (abilities[3] != '-') card.guard = true;
			if (abilities[4] != '-') card.lethal = true;
			if (abilities[5] != '-') card.ward = true;
		}
	}
}

void		Agent::attack(vector<Card> cards)
{
	chooseTarget(cards);

	for (Card &card : cards)
	{
		if (card.location != Location::Mine) continue;
		if (!card.target) continue;
		if (!card.target->guard) continue;

		turn.emplace_back();
		Action &action = turn.back();
		action.attack(card.id, card.target->id);
	}

	for (Card &card : cards)
	{
		if (card.location != Location::Mine) continue;
		if (card.target) continue;

		turn.emplace_back();
		Action &action = turn.back();
		action.attack(card.id);
	}
}

void		Agent::think(void)
{
	turn.clear();

	//Draft
	if (state.player.mana == 0)
	{
		double nb = -1.;
		int best = 0;
		int i = -1;

		for (Card &card : state.cards)
		{
			i++;
			if (card.type == CardType::Blue)
			{
				best = i;
				break;
			}
			if (card.type == CardType::Monster && card.ratio() > nb)
			{
				nb = card.ratio();
				best = i;
			}
		}
		turn.emplace_back();
		Action &action = turn.back();
		action.pick(best);
	}

	else
	{
		//Summon Monster
		{
			Card::sortByCostAndRatio(state.cards);
			for (Card &card : state.cards)
			{
				if (card.location != Location::Hand) continue;
				if (card.type != CardType::Monster) continue;
				if (card.cost > state.player.mana) continue;

				turn.emplace_back();
				Action &action = turn.back();
				action.summon(card.id);

				state.player.mana -= card.cost;
				if (card.charge)
					card.location = Location::Mine;
			}
		}

		//Activate Spell
		for (Card &card : state.cards)
		{
			if (card.location != Location::Hand) continue ;
			if (card.cost > state.player.mana) continue ;

			if (card.type == CardType::Green)
			{
				for (Card &target : state.cards)
				{
					if (target.location != Location::Mine) continue ;

					turn.emplace_back();
					Action &action = turn.back();
					action.use(card.id, target.id);
					state.player.mana -= card.cost;
				}
			}

			else if (card.type == CardType::Red)
			{
				 for (Card &target : state.cards)
				 {
					if (target.location != Location::Enemy) continue;

					turn.emplace_back();
					Action &action = turn.back();
					action.use(card.id, target.id);
					state.player.mana -= card.cost;
				 }
			}

			else if (card.type == CardType::Blue)
			{
				 turn.emplace_back();
				 Action &action = turn.back();
				 action.use(card.id);
				 state.player.mana -= card.cost;
			}
		}

		//Battle
		attack(state.cards);
	}
}

void		Agent::play(void)
{
	for (Action &action : turn)
		action.print(cout);
	cout << endl ;
}

int main()
{
	Agent agent;
	while (1)
	{
		agent.read();
		agent.think();
		agent.play();
	}
}