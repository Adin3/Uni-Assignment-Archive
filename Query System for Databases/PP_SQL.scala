import scala.language.implicitConversions

trait PP_SQL_DB{
  def eval: Option[Database]
}

case class CreateTable(database: Database, tableName: String) extends PP_SQL_DB{
  def eval: Option[Database] = {
    if (database == null) None
    else Some(database.create(tableName))
  }
}

case class DropTable(database: Database, tableName: String) extends PP_SQL_DB{
  def eval: Option[Database] = {
    if (database == null) None
    else Some(database.drop(tableName))
  }
}

implicit def PP_SQL_DB_Create_Drop(t: (Option[Database], String, String)): Option[PP_SQL_DB] = {
  t._1 match {
    case None => None
    case Some(db) => t._2 match {
      case "CREATE" => Some(CreateTable(db, t._3))
      case "DROP" => Some(DropTable(db, t._3))
    }
  }
}

case class SelectTables(database: Database, tableNames: List[String]) extends PP_SQL_DB{
  def eval: Option[Database] = {
    if (database == null) None
    else database.selectTables(tableNames)
  }
}

implicit def PP_SQL_DB_Select(t: (Option[Database], String, List[String])): Option[PP_SQL_DB] = {
  t._1 match {
    case None => None
    case Some(db) => Some(SelectTables(db, t._3))
  }
}

case class JoinTables(database: Database, table1: String, column1: String, table2: String, column2: String) extends PP_SQL_DB{
  def eval: Option[Database] = {
    if (database == null) None
    else database.join(table1, column1, table2, column2) match {
      case None => None
      case Some(x) => Some(Database(List(x)))
    }
  }
}

implicit def PP_SQL_DB_Join(t: (Option[Database], String, String, String, String, String)): Option[PP_SQL_DB] = {
  t._1 match {
    case None => None
    case Some(db) => Some(JoinTables(db, t._3, t._4, t._5, t._6))
  }
}


trait PP_SQL_Table{
  def eval: Option[Table]
}

case class InsertRow(table:Table, values: Tabular) extends PP_SQL_Table{
  def eval: Option[Table] = {
    if (table == null) None
    else Some(values.foldRight(table)((xs, x) => x.insert(xs)))
  }
}

implicit def PP_SQL_Table_Insert(t: (Option[Table], String, Tabular)): Option[PP_SQL_Table] = {
  t._1 match {
    case None => None
    case Some(tb) => Some(InsertRow(tb, t._3))
  }
}

case class UpdateRow(table: Table, condition: FilterCond, updates: Map[String, String]) extends PP_SQL_Table{
  def eval: Option[Table] = {
    if (table == null) None
    else Some(table.update(condition, updates))
  }
}

implicit def PP_SQL_Table_Update(t: (Option[Table], String, FilterCond, Map[String, String])): Option[PP_SQL_Table] = {
  t._1 match {
    case None => None
    case Some(tb) => Some(UpdateRow(tb, t._3, t._4))
  }
}

case class SortTable(table: Table, column: String) extends PP_SQL_Table{
  def eval: Option[Table] = {
    if (table == null) None
    else Some(table.sort(column))
  }
}

implicit def PP_SQL_Table_Sort(t: (Option[Table], String, String)): Option[PP_SQL_Table] = {
  t._1 match {
    case None => None
    case Some(tb) => Some(SortTable(tb, t._3))
  }
}

case class DeleteRow(table: Table, row: Row) extends PP_SQL_Table{
  def eval: Option[Table] = {
    if (table == null) None
    else Some(table.delete(row))
  }
}

implicit def PP_SQL_Table_Delete(t: (Option[Table], String, Row)): Option[PP_SQL_Table] = {
  t._1 match {
    case None => None
    case Some(tb) => Some(DeleteRow(tb, t._3))
  }
}

case class FilterRows(table: Table, condition: FilterCond) extends PP_SQL_Table{
  def eval: Option[Table] = {
    if (table == null) None
    else Some(table.filter(condition))
  }
}

implicit def PP_SQL_Table_Filter(t: (Option[Table], String, FilterCond)): Option[PP_SQL_Table] = {
  t._1 match {
    case None => None
    case Some(tb) => Some(FilterRows(tb, t._3))
  }
}

case class SelectColumns(table: Table, columns: List[String]) extends PP_SQL_Table{
  def eval: Option[Table] = {
    if (table == null) None
    else Some(table.select(columns))
  }
}

implicit def PP_SQL_Table_Select(t: (Option[Table], String, List[String])): Option[PP_SQL_Table] = {
  t._1 match {
    case None => None
    case Some(tb) => Some(SelectColumns(tb, t._3))
  }
}

def queryT(p: Option[PP_SQL_Table]): Option[Table] = {
  p match {
    case None => None
    case Some(tb) => tb.eval
  }
}
def queryDB(p: Option[PP_SQL_DB]): Option[Database] = {
  p match {
    case None => None
    case Some(db) => db.eval
  }
}