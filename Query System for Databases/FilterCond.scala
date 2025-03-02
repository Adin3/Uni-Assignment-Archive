import scala.language.implicitConversions

trait FilterCond {def eval(r: Row): Option[Boolean]}

case class Field(colName: String, predicate: String => Boolean) extends FilterCond {
  override def eval(r: Row): Option[Boolean] = {
    r.get(colName) match {
      case None => None
      case Some(value) => Some(predicate(value))
    }
  }
}

case class Compound(op: (Boolean, Boolean) => Boolean, conditions: List[FilterCond]) extends FilterCond {
  override def eval(r: Row): Option[Boolean] = {
    conditions.tail.foldRight(conditions.head.eval(r))((cond, save) => {
      cond.eval(r) match {
        case None => None
        case Some(c) => Some(op(c, save.get))
      }
    })
  }
}

case class Not(f: FilterCond) extends FilterCond {
  override def eval(r: Row): Option[Boolean] = {
    f.eval(r) match {
      case None => None
      case Some(value) => Some(!value)
    }
  }
}

def And(f1: FilterCond, f2: FilterCond): FilterCond = {
  Compound((_ && _), List(f1, f2))
}
def Or(f1: FilterCond, f2: FilterCond): FilterCond = {
  Compound((_ || _), List(f1, f2))
}
def Equal(f1: FilterCond, f2: FilterCond): FilterCond = {
  Compound((_ == _), List(f1, f2))
}

case class Any(fs: List[FilterCond]) extends FilterCond {
  override def eval(r: Row): Option[Boolean] = Compound((_ || _), fs).eval(r)
}

case class All(fs: List[FilterCond]) extends FilterCond {
  override def eval(r: Row): Option[Boolean] = Compound((_ && _), fs).eval(r)
}

implicit def tuple2Field(t: (String, String => Boolean)): Field = Field(t._1, t._2)

extension (f: FilterCond) {
  def ===(other: FilterCond) = Equal(f, other)
  def &&(other: FilterCond) = And(f, other)
  def ||(other: FilterCond) = Or(f, other)
  def !! = Not(f)
}