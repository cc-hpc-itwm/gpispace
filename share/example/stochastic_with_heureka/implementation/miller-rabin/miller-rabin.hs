-- http://en.wikipedia.org/wiki/Miller-Rabin_primality_test

module Main (main) where

import qualified Data.List (genericTake)
import qualified System.Environment (getArgs)
import qualified System.Random (mkStdGen, randomRs)
import qualified Test.QuickCheck (Property, (==>))

factor_out_powers_of_two :: Integer -> (Integer, Integer)
factor_out_powers_of_two n = div2 0 (n - 1)
  where div2 s m = let (q,r) = divMod m 2
                   in case r of 0 -> div2 (s + 1) q
                                1 -> (s, m)

property_factor_out_powers_of_two :: Integer -> Test.QuickCheck.Property
property_factor_out_powers_of_two n =
  (and [n > 3, odd n])
  Test.QuickCheck.==> let (s, d) = factor_out_powers_of_two n
                      in n == 2^s * d + 1

mod_pow :: Integer -> Integer -> Integer -> Integer
mod_pow _ _ 1 = 0
mod_pow _ 0 _ = 1
mod_pow a 1 n = mod a n
mod_pow a d n = let (q, r) = divMod d 2
                    h = mod_pow a q n
                    p = h * h
                in mod (if r == 0 then p else p * a) n

property_mod_pow :: Integer -> Integer -> Integer -> Test.QuickCheck.Property
property_mod_pow a d n =
  and [a >= 0, d >= 0, n > 0] Test.QuickCheck.==> mod (a^d) n == mod_pow a d n

is_witness_for_compositeness :: Integer -> Integer -> Bool
is_witness_for_compositeness n a
   | x == 1 || x == n - 1 = False
   | otherwise = check xs
   where (s, d) = factor_out_powers_of_two n
         x = mod_pow a d n
         xs = Data.List.genericTake s $ iterate (\ v -> mod (v * v) n) x
         check [] = True
         check (v:vs) | v == 1 = True
                      | v == n - 1 = False
                      | otherwise = check vs

witness :: Int -> Integer -> Integer -> Bool
witness seed n k = or
                 $ map (is_witness_for_compositeness n)
                 $ Data.List.genericTake k
                 $ System.Random.randomRs (2, n - 2)
                 $ System.Random.mkStdGen seed

main :: IO ()
main = System.Environment.getArgs >>= \ (seed:n:k:_) ->
       case witness (read seed) (read n) (read k) of
         False -> putStrLn "probably prime"
         True -> putStrLn "composite"

{--
rahn@brank:~$ ghc --make -O2 miller-rabin.hs
rahn@brank:~$ /usr/bin/time -f "elapsed %e" ./miller-rabin 3141 32212254719 100000
probably prime
elapsed 0.90
rahn@brank:~$ /usr/bin/time -f "elapsed %e" ./miller-rabin 3141 2833419889721787128217599 100000
probably prime
elapsed 3.17
rahn@brank:~$ /usr/bin/time -f "elapsed %e" ./miller-rabin 3141 91270843213599097018040811446599681 100000
composite
elapsed 0.00
--}
