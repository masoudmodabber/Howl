class AttackPlaces
{
public:
    static long long WhitePawnAttackPlaces[64];
    static long long BlackPawnAttackPlaces[64];
    static long long KnightAttackPlaces[64];
    static long long KingAttackPlaces[64];
    static long long BishopAttack[64][64];
    static long long RookAttack[64][64];
    static long long QueenAttack[64][64];

    static void Initialize();
    static void Cleanup();

    static void SetPawnAttackPlaces();
    static void SetKingAttackPlaces();
    static void SetKnightAttackPlaces();
    static void SetBishopAttackPlaces();
    static void SetRookAttackPlaces();
    static void SetQueenAttackPlaces();
};
