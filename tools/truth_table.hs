import qualified Data.List (nub)

data Exp = Const Bool
         | Var String
         | And [Exp]
         | Or [Exp]
         | Not Exp
         | Implies Exp Exp
         deriving (Eq, Ord, Show)

varsL (Const _) = []
varsL (Var v) = [v]
varsL (And es) = concatMap vars es
varsL (Or es) = concatMap vars es
varsL (Not e) = vars e
varsL (Implies l r) = vars l ++ vars r

vars = Data.List.nub . varsL

value _ (Const v) = v
value f (Var v) = f v
value f (And es) = and $ map (value f) es
value f (Or es) = or $ map (value f) es
value f (Not e) = not $ value f e
value f (Implies l r) = or [value f (Not l), value f r]

assignments [] = [const True]
assignments (x:xs) =
  [ \ v -> if v == x then True else f v | f <- assignments xs ]
  ++
  [ \ v -> if v == x then False else f v | f <- assignments xs ]

disproofs e =
  let vs = vars e
      fs = assignments vs
      nice f = zip vs $ map (value f . Var) vs
  in map snd $ filter (not . fst) $ zip (map (flip value e) fs) (map nice fs)

is_tautology = null . disproofs

exp_04e69c70d47d0415b01d7309b8f650f1d5057b58_msg
 = let l = Not $ Or [Var "x", Not $ And [Var "a", Var "b", Var "c"]]
       r = And [Var "x", Var "a", Var "b", Var "c"]
   in And [Implies l r, Implies r l]

exp_04e69c70d47d0415b01d7309b8f650f1d5057b58_impl
 = let l = Not $ Or [Var "x", Not $ And [Var "a", Var "b", Var "c"]]
       r = And [Not $ Var "x", Var "a", Var "b", Var "c"]
   in And [Implies l r, Implies r l]
