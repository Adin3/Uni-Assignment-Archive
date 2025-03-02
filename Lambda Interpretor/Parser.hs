module Parser (parseLambda, parseLine) where

import Control.Monad
import Control.Applicative

import Lambda
import Binding

import Data.Char


newtype Parser a = Parser { parse :: String -> Maybe (a, String) }

failParser :: Parser a
failParser = Parser $ const Nothing

charParser :: Char -> Parser Char
charParser c = Parser $
  \s -> case s of
           [] -> Nothing
           (x:xs) -> if x == c then Just ((c,xs)) else Nothing

predicateParser :: (Char -> Bool) -> Parser Char
predicateParser p = Parser $
  \s -> case s of
           [] -> Nothing
           (x:xs) -> if p x then Just ((x,xs)) else Nothing

instance Monad Parser where
  mp >>= f = Parser $
    \s -> case parse mp s of
        Nothing -> Nothing
        Just (val, rest) -> parse (f val) rest
  
  return x = Parser $ \s -> Just(x,s)

instance Applicative Parser where
  af <*> mp =
    do
      f <- af
      v <- mp
      return $ f v
  pure = return

instance Functor Parser where
  fmap f mp =
    do
      x <- mp
      return $ f x

instance Alternative Parser where
  empty = failParser
  p1 <|> p2 = Parser $ \s -> case parse p1 s of
                      Nothing -> parse p2 s
                      x -> x

plusParser :: (Parser a) -> Parser [a]
plusParser p =
  do
    x <- p
    xs <- starParser p
    return $ x:xs

starParser :: (Parser a) -> Parser [a]
starParser p = (plusParser p) <|> return []

isLowerNum :: Char -> Bool
isLowerNum x= isLower x || isDigit x

varParser :: Parser String 
varParser = 
  do x <- predicateParser isLower
     xs <- starParser (predicateParser isLowerNum)
     return (x:xs)

isUpperNum :: Char -> Bool
isUpperNum x= isUpper x || isDigit x

macroParser :: Parser String 
macroParser = 
  do x <- predicateParser isUpperNum
     xs <- starParser (predicateParser isUpperNum)
     return (x:xs)

varLambdaParser :: Parser Lambda
varLambdaParser = Var <$> varParser

macroLambdaParser :: Parser Lambda
macroLambdaParser = Macro <$> macroParser

elemLambdaParser :: Parser Lambda
elemLambdaParser = varLambdaParser <|> macroLambdaParser

whitespaceParser :: Parser String
whitespaceParser = starParser (charParser ' ')

lambdaAppParser :: Parser Lambda
lambdaAppParser = 
  do 
     charParser '('
     e1 <- lambdaParser
     whitespaceParser
     e2 <- lambdaParser
     charParser ')'
     return $ App e1 e2

lambdaAbsParser :: Parser Lambda
lambdaAbsParser = 
  do 
     charParser '\\'
     v <- varParser
     charParser '.'
     e2 <- lambdaParser
     return $ Abs v e2

lambdaParser :: Parser Lambda
lambdaParser = lambdaAbsParser <|> lambdaAppParser <|> elemLambdaParser

bindingParser :: Parser Line
bindingParser = 
    do
        m <- macroParser
        charParser '='
        e <- lambdaParser
        return $ Binding m e

evalParser = 
    do
        l <- lambdaParser
        return $ Eval l

lineParser :: Parser Line
lineParser = bindingParser <|> evalParser

-- 2.1. / 3.2.
parseLambda :: String -> Lambda
parseLambda s = 
  case parse lambdaParser s of 
        Just (e,"") -> e
        _ -> Macro "ERROR"

-- 3.3.
parseLine :: String -> Either String Line
parseLine s = 
    case parse lineParser s of
        Just (line, "") -> case line of
            (Eval l) -> case l of
                Macro "ERROR" -> Left "ERROR"
                _             -> Right $ Eval l
            (Binding v l) -> case l of
                Macro "ERROR" -> Left "ERROR"
                _             -> Right $ Binding v l
        _ -> Left "ERROR"


