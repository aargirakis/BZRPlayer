/**
 * @file
 * @brief Caching catalog implementation
 * @author vitamin.caig@gmail.com
 */
package app.zxtune.fs.zxtunes

import app.zxtune.Logger
import app.zxtune.TimeStamp
import app.zxtune.fs.dbhelpers.CommandExecutor
import app.zxtune.fs.dbhelpers.QueryCommand

private val AUTHORS_TTL = TimeStamp.fromDays(30)
private val TRACKS_TTL = TimeStamp.fromDays(7)

private val LOG = Logger(CachingCatalog::class.java.name)

class CachingCatalog internal constructor(
    private val remote: Catalog,
    private val db: Database
) : Catalog {
    private val executor: CommandExecutor = CommandExecutor("zxtunes")

    override fun queryAuthors(visitor: Catalog.AuthorsVisitor) =
        executor.executeQuery("authors", object : QueryCommand {
            private val lifetime = db.getAuthorsLifetime(AUTHORS_TTL)

            override val isCacheExpired: Boolean
                get() = lifetime.isExpired

            override fun updateCache() = db.runInTransaction {
                remote.queryAuthors(db::addAuthor)
                lifetime.update()
            }

            override fun queryFromCache() = db.queryAuthors(visitor)
        })

    override fun queryAuthorTracks(author: Author, visitor: Catalog.TracksVisitor) =
        executor.executeQuery("tracks", object : QueryCommand {
            private val lifetime = db.getAuthorTracksLifetime(author, TRACKS_TTL)

            override val isCacheExpired: Boolean
                get() = lifetime.isExpired

            override fun updateCache() = db.runInTransaction {
                remote.queryAuthorTracks(author) { obj ->
                    db.addTrack(obj)
                    db.addAuthorTrack(author, obj)
                }
                lifetime.update()
            }

            override fun queryFromCache() = db.queryAuthorTracks(author, visitor)
        })

    //TODO: rework logic to more clear
    override fun searchSupported() = if (remote.searchSupported()) {
        //network is available, so direct scanning may be more comprehensive
        //TODO: check out if all the cache is not expired
        LOG.d { "Disable fast search due to enabled network" }
        false
    } else {
        true
    }

    //TODO: query also remote catalog when search will be enabled
    override fun findTracks(query: String, visitor: Catalog.FoundTracksVisitor) =
        db.findTracks(query, visitor)
}
