#include <QtDebug>
#include <QtTest/QtTest>
#include <QDir>
#include <QFileInfo>

#include "Core/Task.h"
#include "Core/CharmConstants.h"
#include "Core/CharmExceptions.h"

#include "TaskStructureTests.h"

TaskStructureTests::TaskStructureTests()
    : QObject()
{
}

QList<QDomElement> TaskStructureTests::retrieveTestCases( QString path, QString type )
{
    const QString tagName( "testcase" );
    QStringList filenamePatterns;
    filenamePatterns << "*.xml";

    QDir dataDir( path );
    if ( !dataDir.exists() ) {
        throw CharmException( "path to test case data does not exist" );
    }

    QFileInfoList dataSets = dataDir.entryInfoList( filenamePatterns, QDir::Files, QDir::Name );

    QList<QDomElement> result;
    Q_FOREACH( QFileInfo fileinfo, dataSets ) {
        QDomDocument doc( "charmtests" );
        QFile file( fileinfo.filePath() );
        if ( ! file.open( QIODevice::ReadOnly ) ) {
            throw CharmException( "unable to open input file" );
        }

        if ( !doc.setContent( &file ) ) {
            throw CharmException( "invalid DOM document, cannot load" );
        }

        QDomElement root = doc.firstChildElement();
        if ( root.tagName() != "testcases" ) {
            throw CharmException( "root element (testcases) not found" );
        }

        qDebug() << "Loading test case" << file.fileName();

        for ( QDomElement child = root.firstChildElement( tagName ); !child.isNull();
              child = child.nextSiblingElement( tagName ) ) {
            if ( child.attribute( "type" ) == type ) {
                result << child;
            }
        }
    }
    return result;
}

void TaskStructureTests::checkForUniqueTaskIdsTest_data()
{
    QTest::addColumn<TaskList>( "tasks" );
    QTest::addColumn<bool>( "unique" );
    const QString tasksTagName( "tasks" );

    Q_FOREACH( QDomElement testcase,
               retrieveTestCases( ":/checkForUniqueTaskIdsTest/Data", "checkForUniqueTaskIdsTest" ) ) {
        Q_ASSERT( testcase.attribute( "type" ) == "checkForUniqueTaskIdsTest" );

        QString name = testcase.attribute( "name" );
        QString expectedResultText = testcase.attribute( "expectedResult" );
        QVERIFY( expectedResultText == "false" || expectedResultText == "true" );
        bool expectedResult = ( expectedResultText == "true" );
        QDomElement element = testcase.firstChildElement( tasksTagName );
        QVERIFY( !element.isNull() );
        TaskList tasks = Task::readTasksElement( element, CHARM_DATABASE_VERSION );
        QTest::newRow( name.toLocal8Bit() ) << tasks << expectedResult;
        QVERIFY( element.nextSiblingElement( tasksTagName ).isNull() );
        qDebug() << "Added test case" << name;
    }
}

void TaskStructureTests::checkForUniqueTaskIdsTest()
{
    QFETCH( TaskList, tasks );
    QFETCH( bool,  unique );

    QCOMPARE( Task::checkForUniqueTaskIds( tasks ), unique );
}

void TaskStructureTests::checkForTreenessTest_data()
{
    QTest::addColumn<TaskList>( "tasks" );
    QTest::addColumn<bool>( "directed" );

    // FIXME this will be read from resources:
    // set up test candidates:
    TaskList tasks;
    Task task1;
    task1.setName( "A task" );
    task1.setId( 1 );
    task1.setParent( 0 );
    task1.setSubscribed( true );
    task1.setValidFrom( QDateTime::currentDateTime() );
    Task task2;
    task2.setName( "Another task" );
    task2.setId( 2 );
    task2.setParent( 1 );
    task2.setSubscribed( false );
    task2.setValidUntil( QDateTime::currentDateTime() );

    tasks << task1 << task2;
    QTest::newRow( "Empty TaskList" ) << TaskList() << true;
    QTest::newRow( "Simple List" ) << tasks << true;

    // now add a cycle to the graph, and make it fail:
    Task task3;
    task3.setName( "3" );
    task3.setId( 3 );
    task3.setParent( 5 );
    Task task4;
    task4.setName( "4" );
    task4.setId( 4 );
    task4.setParent( 3 );
    Task task5;
    task5.setName( "5" );
    task5.setId( 5 );
    task5.setParent( 4 );
    TaskList cyclicTaskList;
    cyclicTaskList << task3 << task4 << task5;
    QTest::newRow( "Cyclic TaskList" ) << cyclicTaskList << false;

    // add a task list of just one task that is it's own child:
    Task task6;
    task6.setId( 6 );
    task6.setParent( 6 );
    task6.setName( "6" );
    TaskList oneTask;
    oneTask << task6;
    QTest::newRow( "One cyclic task" ) << oneTask << false;
}

void TaskStructureTests::checkForTreenessTest()
{
    QFETCH( TaskList, tasks );
    QFETCH( bool, directed );

    QCOMPARE( Task::checkForTreeness( tasks ), directed );
}

void TaskStructureTests::mergeTaskListsTest()
{
    qDebug() << "NI";
}

QTEST_MAIN( TaskStructureTests )

#include "TaskStructureTests.moc"


