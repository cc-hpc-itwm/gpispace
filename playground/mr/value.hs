-- mirko.rahn@itwm.fraunhofer.de

import qualified Data.Map ( Map, empty, insert, lookup, isSubmapOfBy, assocs
                          , map, fromList
                          )
import qualified Data.Set (Set, empty, insert)
import qualified Data.Maybe (maybe)
import qualified Data.List (intersperse)

data Value = Control
           | Int Int
           | String [Char]
           | Map (Data.Map.Map Value Value)
           | Set (Data.Set.Set Value)
           | Struct (Data.Map.Map String Value)
           deriving (Eq, Ord, Read, Show)

empty :: Value
empty = Struct Data.Map.empty

get :: [String] -> Value -> Maybe Value
get [] v = return v
get (key:keys) (Struct m) = Data.Map.lookup key m >>= get keys
get _ _ = Nothing

put :: [String] -> Value -> Value -> Value
put [] x _ = x
put (key:keys) x (Struct m) = Struct $ Data.Map.insert key (put keys x v) m
  where v = Data.Maybe.maybe empty id $ Data.Map.lookup key m
put keys x _ = put keys x empty

same_type :: Value -> Value -> Bool
same_type Control Control = True
same_type (Int _) (Int _) = True
same_type (String _) (String _) = True
same_type (Map _) (Map _) = True
same_type (Set _) (Set _) = True
same_type (Struct l) (Struct r) = and [ Data.Map.isSubmapOfBy same_type l r
                                      , Data.Map.isSubmapOfBy same_type r l
                                      ]
same_type _ _ = False

type Signature = Value

signature :: Value -> Signature
signature = id

-- note that the leaf values are never evaluated
show_as_signature :: Signature -> String
show_as_signature Control = "Control"
show_as_signature (Int _) = "Int"
show_as_signature (String _) = "String"
show_as_signature (Map _) = "Map"
show_as_signature (Set _) = "Set"
show_as_signature (Struct m) = "[" ++ concat childs ++ "]"
  where childs = Data.List.intersperse ", "
                 [ k ++ " :: " ++ show_as_signature v
                 | (k,v) <- Data.Map.assocs m
                 ]

default_from_signature :: Signature -> Value
default_from_signature Control = Control
default_from_signature (Int _) = Int 0
default_from_signature (String _) = String ""
default_from_signature (Map _) = Map Data.Map.empty
default_from_signature (Set _) = Set Data.Set.empty
default_from_signature (Struct m) = Struct
                                  $ Data.Map.map default_from_signature m

-- undefined if value has more fields than signature or fields with
-- different type
default_missing_fields :: Signature -> Value -> Value
default_missing_fields Control Control = Control
default_missing_fields (Int _) (Int v) = Int v
default_missing_fields (String _) (String s) = String s
default_missing_fields (Map _) (Map m) = Map m
default_missing_fields (Set _) (Set s) = Set s
default_missing_fields (Struct s) (Struct m)
  | not (Data.Map.isSubmapOfBy (\_ _ -> True) m s) = undefined
default_missing_fields (Struct s) (Struct m) = Struct $ Data.Map.fromList $
  Data.Map.assocs s >>= \ (k,vs) -> case Data.Map.lookup k m of
    Just vm -> [(k, default_missing_fields vs vm)]
    Nothing -> [(k, default_from_signature vs)]
default_missing_fields _ _ = undefined

------------------------------------------------------------------------------

values :: [Value]
values = [ {-  0 -} Control
         , {-  1 -} Control
         , {-  2 -} String "string"
         , {-  3 -} default_from_signature $ put ["key1"] (Int undefined) empty
         , {-  4 -} Set $ foldl (flip Data.Set.insert) Data.Set.empty (take 4 values)
         , {-  5 -} put ["key2"] (values !! 3) empty
         , {-  6 -} put ["0","1"] (values !! 4) empty
         , {-  7 -} put ["0","2"] (values !! 5) (values !! 6)
         , {-  8 -} put ["key1"] (String "x") empty
         , {-  9 -} put ["key2"] (String "y") empty
         , {- 10 -} put ["key1"] (String "z") (values !! 9)
         , {- 11 -} put ["key2"] (String "z") (values !! 8)
         ]

tests_okay :: Bool
tests_okay = and $ map and [tests_get, tests_same_type, tests_sig]

tests_get :: [Bool]
tests_get =
  [ get [] (values !! 0) == Just Control
  , get [""] (values !! 0) == Nothing
  , get [] (values !! 3) == Just (values !! 3)
  , get ["key1"] (values !! 0) == Nothing
  , get ["key1"] (values !! 3) == Just (default_from_signature $ Int undefined)
  , get ["key2"] (values !! 3) == Nothing
  , get ["key1"] (values !! 5) == Nothing
  , get ["key2"] (values !! 5) == Just (values !! 3)
  , get ["0","1"] (values !! 6) == Just (values !! 4)
  , get ["0","2"] (values !! 6) == Nothing
  , get ["0","2"] (values !! 7) == Just (values !! 5)
  , get ["0","2","key2"] (values !! 7) == Just (values !! 3)
  , get ["0","2","key1"] (values !! 7) == Nothing
  , get ["0","2","key2","key1"] (values !! 7) == Just (default_from_signature $ Int undefined)
  ]

tests_same_type :: [Bool]
tests_same_type =
  [ same_type (Int undefined) (Int undefined) == True
  , same_type (String undefined) (String undefined) == True
  , same_type (Map undefined) (Map undefined) == True
  , same_type (Set undefined) (Set undefined) == True
  , same_type empty empty == True
  , same_type (values !! 7) (values !! 7) == True
  , same_type (Int undefined) (String undefined) == False
  , same_type (values !! 8) (values !! 3) == False
  , same_type (values !! 8) (values !! 9) == False
  , same_type (values !! 8) (values !! 9) == False
  , same_type (values !! 10) (values !! 11) == True
  , (==) (values !! 10) (values !! 11) == False
  ]

sig :: Signature
sig = put ["x"] (Set undefined)
    $ put ["a","c"] (String undefined)
    $ put ["a","b"] (Int undefined) empty

val :: Value
val = put ["a","c"] (String "s") empty

tests_sig :: [Bool]
tests_sig =
  [ default_missing_fields sig val
  == ( put ["x"] (default_from_signature $ Set undefined)
     $ put ["a","c"] (String "s")
     $ put ["a","b"] (default_from_signature $ Int undefined) empty
     )
  ]

-- undefined == default_missing_fields sig $ put ["b"] (String "s") empty
-- undefined == default_missing_fields sig $ put ["a","c"] (Int 0) empty
