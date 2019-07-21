#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <key_helper.h>
#include "signalprotocol.h"
#include "omemostore.h"

#define CONNECTION_NAME "OMEMO"

#define INIT_STATUS "init_status"
#define OWN_PUBLIC_KEY "own_public_key"
#define OWN_PRIVATE_KEY "own_private_key"
#define OWN_KEY 2
#define REG_ID "axolotl_registration_id"
#define IDENTITY_KEY_TRUSTED 1
#define IDENTITY_KEY_UNTRUSTED 1

#define SESSION_STORE_TABLE "session_store"
#define SESSION_STORE_NAME "name"
#define SESSION_STORE_DEVICE_ID "device_id"
#define SESSION_STORE_RECORD "session_record"
#define PRE_KEY_STORE_TABLE "pre_key_store"
#define PRE_KEY_STORE_ID "id"
#define PRE_KEY_STORE_RECORD "pre_key_record"
#define SIGNED_PRE_KEY_STORE_TABLE "signed_pre_key_store"
#define SIGNED_PRE_KEY_STORE_ID "id"
#define SIGNED_PRE_KEY_STORE_RECORD "signed_pre_key_record"
#define IDENTITY_KEY_STORE_TABLE "identity_key_store"
#define IDENTITY_KEY_STORE_NAME "name"
#define IDENTITY_KEY_STORE_KEY "key"
#define IDENTITY_KEY_STORE_TRUSTED "trusted"
#define SETTINGS_STORE_TABLE "settings"
#define SETTINGS_STORE_NAME "name"
#define SETTINGS_STORE_PROPERTY "property"

#define SQL_QUERY(Q) QSqlQuery query(QString(Q), db())

namespace OmemoStore
{
// Session store implementation
void addDatabase(const QString &ADatabaseFileName, const QString &ADatabaseName)
{
	qDebug("OmemoStore::init(\"%s\")", ADatabaseFileName.toUtf8().data());
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", ADatabaseName);
	db.setDatabaseName(ADatabaseFileName);
	if (db.open())
		qDebug("Database opened!");
	else
		qCritical("Database open failed!");
}

void uninit()
{
	qDebug("OmemoStore::uninit()");
	QSqlDatabase::database(CONNECTION_NAME).close();
	QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

QSqlDatabase db()
{
	return QSqlDatabase::database(CONNECTION_NAME);
}

int propertySet(const char * name, const int val)
{
	qDebug("OmemoStore::propertySet(%s, %d)", name, val);
	// 1 - name of property
	// 2 - value
	SQL_QUERY("INSERT OR REPLACE INTO " SETTINGS_STORE_TABLE " VALUES (?1, ?2)");

	query.bindValue(0, name);
	query.bindValue(1, val);

	if (!query.exec()) return -3;

	return 0;
}

int propertyGet(const char * name, int * val_p)
{
	qDebug("OmemoStore::propertyGet(%s, %p)", name, (void *)val_p);
	SQL_QUERY("SELECT * FROM " SETTINGS_STORE_TABLE " WHERE name IS ?1");

	query.bindValue(0, name);

	if (query.exec())	{
		if (query.next()) {
			const int temp = query.value(1).toInt();
			// exactly one result
			if (query.next()) {
				qCritical("Too many results");
				return -3;
			}
			*val_p = temp;
			return 0;
		} else {
			qCritical("Result not found");
			return 1;
		}
	} else
		return -3;
}

int sessionLoad(signal_buffer ** record,
				signal_buffer ** user_record,
				const signal_protocol_address * address,
				void * user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(user_record);

	SQL_QUERY("SELECT * FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1"
			  " AND " SESSION_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(address));
	query.bindValue(1, address->device_id);

	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(2).toByteArray();
			*record = signal_buffer_create(DATA_SIZE(data));
			if (*record == nullptr) {
			  qCritical("Buffer could not be initialised");
			  return -3;
			}
		} else {
			// session not found
			return 0;
		}
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}

	return 1;
}

int sessionGetSubDeviceSessions(signal_int_list ** sessions,
								const char * name, size_t name_len,
								void * user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(name_len);

	SQL_QUERY("SELECT * FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1");

	signal_int_list * session_list_p = nullptr;
	int ret_val = 0;

	query.bindValue(0, name);
	if (!query.isValid()) {
		qCritical("Failed to bind name when trying to find sub device sessions");
		ret_val = -21;
		goto cleanup;
	}

	session_list_p = signal_int_list_alloc();

	if (query.exec()) {
		while (query.next())
			signal_int_list_push_back(session_list_p, query.value(2).toInt());
	} else {
		qCritical("Error while retrieving result rows");
		ret_val = -3;
		goto cleanup;
	}

	*sessions = session_list_p;
	ret_val = int(signal_int_list_size(*sessions));

cleanup:
	if (ret_val < 0) {
		if (session_list_p) {
		  signal_int_list_free(session_list_p);
		}
	}

	return ret_val;
}

int sessionStore(const signal_protocol_address *address,
				 uint8_t *record, size_t record_len,
				 uint8_t *user_record, size_t user_record_len,
				 void *user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(user_record);
	Q_UNUSED(user_record_len);

	SQL_QUERY("INSERT OR REPLACE INTO " SESSION_STORE_TABLE
			  " VALUES (:name, :device_id, :session_record)");

	query.bindValue(0, ADDR_NAME(address));
	query.bindValue(1, address->device_id);
	query.bindValue(2, BYTE_ARRAY(record, record_len));
	if (!query.exec())
	{
		qCritical("QSL query execution failed: \"%s\"; rc=%d (%s)",
					query.lastQuery().toUtf8().data(),
					query.lastError().number(),
					query.lastError().text().toUtf8().data());
		return -3;
	}

	return 0;
}

int sessionContains(const signal_protocol_address *address, void *user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("SELECT * FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1"
			  " AND " SESSION_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(address));
	query.bindValue(1, address->device_id);

	if (query.exec()) {
		if (query.next())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}
}

int sessionDelete(const signal_protocol_address *address, void *user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("DELETE FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1"
			  " AND " SESSION_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(address));
	query.bindValue(1, address->device_id);

	if (query.exec()) {
		if (query.numRowsAffected())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed to delete session");
		return -4;
	}
}

int sessionDeleteAll(const char *name, size_t name_len, void *user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(name_len);

	SQL_QUERY("DELETE FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1");

	query.bindValue(0, QString(name));

	if (query.exec())
		return query.numRowsAffected();
	else {
		qCritical("Failed to delete sessions");
		return -4;
	}
}

void sessionDestroyStoreCtx(void *user_data)
{
	Q_UNUSED(user_data);
}

// pre key store impl
int preKeyLoad(signal_buffer ** record, uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("SELECT * FROM " PRE_KEY_STORE_TABLE
			  " WHERE " PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, pre_key_id);

	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(1).toByteArray();
			*record = signal_buffer_create(DATA_SIZE(data));
			if (!*record) {
				qCritical("Buffer could not be initialised");
				return -3;
			}
		} else {
			qCritical("Pre key ID not found");
			return SG_ERR_INVALID_KEY_ID;
		}
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}

	return SG_SUCCESS;
}

int preKeyStore(uint32_t pre_key_id, uint8_t * record, size_t record_len, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE
			  " VALUES (?1, ?2)");

	query.bindValue(0, pre_key_id);
	query.bindValue(1, BYTE_ARRAY(record,record_len));
	if (!query.exec())
	{
		qCritical("query execution failed: \"%s\"; error=%d (%s)",
				  query.lastQuery().toUtf8().data(),
				  query.lastError().number(),
				  query.lastError().text().toUtf8().data());
		return -3;
	}

	qDebug() << "Pre key stored successfuly!";
	return 0;
}

int preKeyContains(uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("SELECT * FROM " PRE_KEY_STORE_TABLE
			  " WHERE " PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, pre_key_id);

	if (query.exec()) {
		if (query.next())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}
}

int preKeyRemove(uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("DELETE FROM " PRE_KEY_STORE_TABLE
			  " WHERE " PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, pre_key_id);

	if (query.exec()) {
		if (query.numRowsAffected()) {
			return 0;
		} else {
			qCritical("Key does not exist");
			return -4;
		}
	} else {
		qCritical("Failed to delete session");
		return -4;
	}
}

void preKeyDestroyCtx(void * user_data)
{
  Q_UNUSED(user_data);
  //const char stmt[] = "DELETE FROM pre_key_store; VACUUM;";

  //db_exec_quick(stmt, user_data);
}

// signed pre key store impl
int signedPreKeyLoad(signal_buffer ** record, uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE
			  " WHERE " SIGNED_PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, signed_pre_key_id);

	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(1).toByteArray();
			*record = signal_buffer_create(DATA_SIZE(data));
			if (!*record) {
				qCritical("Buffer could not be initialised");
				return -3;
			}
		} else {
			// session not found
			return SG_ERR_INVALID_KEY_ID;
		}
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}

	return SG_SUCCESS;
}

int signedPreKeyStore(uint32_t signed_pre_key_id, uint8_t * record, size_t record_len, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("INSERT OR REPLACE INTO " SIGNED_PRE_KEY_STORE_TABLE
			  " VALUES (?1, ?2)");

	query.bindValue(0, signed_pre_key_id);
	query.bindValue(1, BYTE_ARRAY(record, record_len));

	if (!query.exec()) return -3;

	return 0;
}

int signedPreKeyContains(uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE
			  " WHERE " SIGNED_PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, signed_pre_key_id);

	if (query.exec())	{
		if (query.next()) // result exists
			return 1;
		else // no result
			return 0;
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}
}

int signedPreKeyRemove(uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("DELETE FROM " SIGNED_PRE_KEY_STORE_TABLE
			  " WHERE " SIGNED_PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, signed_pre_key_id);

	if (query.exec())	{
		if (query.numRowsAffected())
			return 0;
		else {
			qCritical("Key does not exist");
			return -4;
		}
	} else {
		qCritical("Failed to delete session");
		return -4;
	}
}

void signedPreKeyDestroyCtx(void * user_data)
{
	Q_UNUSED(user_data);
//	const char stmt[] = "DELETE FROM signed_pre_key_store; VACUUM;";

//	db_exec_quick(stmt, user_data);
}


// Identity key store implementation
/**
 * saves the public and private key by using the api serialization calls, as this format (and not the higher-level key type) is needed by the getter.
 */
int identitySetKeyPair(const ratchet_identity_key_pair * key_pair_p) {
	// 1 - name ("public" or "private")
	// 2 - key blob
	// 3 - trusted (1 for true, 0 for false)

	char * err_msg = nullptr;
	int ret_val = 0;
	signal_buffer * pubkey_buf_p = nullptr;
	signal_buffer * privkey_buf_p = nullptr;

	SQL_QUERY("INSERT INTO " IDENTITY_KEY_STORE_TABLE " VALUES (?1, ?2, ?3)");

	// public key
	query.bindValue(0, OWN_PUBLIC_KEY);

	if (ec_public_key_serialize(&pubkey_buf_p,
								ratchet_identity_key_pair_get_public(key_pair_p))) {
		err_msg = "Failed to allocate memory to serialize the public key";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}

	query.bindValue(1, SBUF2BARR(pubkey_buf_p));
	query.bindValue(2, OWN_KEY);
	if (!query.exec()) {
		err_msg = "Failed to execute query";
		ret_val = -3;
		goto cleanup;
	}

	if (query.numRowsAffected() != 1) {
		err_msg = "Failed to insert";
		ret_val = -3;
		goto cleanup;
	}

	// private key
	query.bindValue(0, OWN_PRIVATE_KEY);

	if (ec_private_key_serialize(&privkey_buf_p, ratchet_identity_key_pair_get_private(key_pair_p))) {
		err_msg = "Failed to allocate memory to serialize the private key";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}

	query.bindValue(1, SBUF2BARR(privkey_buf_p));
	if (!query.exec()) {
		err_msg = "Failed to execute query";
		ret_val = -3;
		goto cleanup;
	}

	if (query.numRowsAffected() != 1) {
		err_msg = "Failed to insert";
		ret_val = -3;
		goto cleanup;
	}

cleanup:
	if (pubkey_buf_p) {
		signal_buffer_bzero_free(pubkey_buf_p);
	}

	if (privkey_buf_p) {
		signal_buffer_bzero_free(privkey_buf_p);
	}

	if (err_msg)
		qCritical(err_msg);

	return ret_val;
}

int identityGetKeyPair(signal_buffer ** public_data, signal_buffer ** private_data, void * user_data)
{
	Q_UNUSED(user_data);

	SQL_QUERY("SELECT * FROM " IDENTITY_KEY_STORE_TABLE
			  " WHERE " IDENTITY_KEY_STORE_NAME " IS ?1");

	char * err_msg = nullptr;
	int ret_val = 0;
	signal_buffer * pubkey_buf_p = nullptr;
	signal_buffer * privkey_buf_p = nullptr;

	// public key
	query.bindValue(0, OWN_PUBLIC_KEY);
	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(1).toByteArray();
			pubkey_buf_p = signal_buffer_create(DATA_SIZE(data));
			if (!pubkey_buf_p) {
				err_msg = "Buffer could not be initialised";
				ret_val = -3;
				goto cleanup;
			}
		} else {
			// public key not found
			err_msg = "Own public key not found";
			ret_val = SG_ERR_INVALID_KEY_ID;
			goto cleanup;
		}
	} else {
		err_msg = "Failed executing SQL query";
		ret_val = -3;
		goto cleanup;
	}
//TODO: Check, if we need this
//	query.finish();

	// private key
	query.bindValue(0, OWN_PRIVATE_KEY);

	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(1).toByteArray();
			privkey_buf_p = signal_buffer_create(DATA_SIZE(data));

			if (!privkey_buf_p) {
				err_msg = "Buffer could not be initialised";
				ret_val = -3;
				goto cleanup;
			}
		} else {
			// private key not found
			err_msg = "Own private key not found";
			ret_val = SG_ERR_INVALID_KEY_ID;
			goto cleanup;
		}
	} else {
		err_msg = "Failed executing SQL query";
		ret_val = -3;
		goto cleanup;
	}

	*public_data = pubkey_buf_p;
	*private_data = privkey_buf_p;

cleanup:
	if (ret_val < 0) {
		if (pubkey_buf_p) {
			signal_buffer_bzero_free(pubkey_buf_p);
		}
		if (privkey_buf_p) {
			signal_buffer_bzero_free(privkey_buf_p);
		}
	}

	if (err_msg)
		qCritical(err_msg);
	return ret_val;
}

int identitySetLocalRegistrationId(const uint32_t reg_id, SignalProtocol * axc_ctx_p)
{
	return (propertySet(REG_ID, int(reg_id))) ? -1 : 0;
}

int identityGetLocalRegistrationId(void * user_data, uint32_t * registration_id)
{
	Q_UNUSED(user_data);

	SQL_QUERY("SELECT * FROM " SETTINGS_STORE_TABLE
			  " WHERE " SETTINGS_STORE_NAME " IS ?1");

	query.bindValue(0, REG_ID);

	if (query.exec()) {
		if (query.next()) {
			*registration_id = query.value(1).toUInt();
		} else {
			qCritical("Own registration ID not found");
			return -31;
		}
	} else {
		qCritical("Failed executing SQL query");
		return -32;
	}

	return 0;
}

int identitySave(const signal_protocol_address * addr_p, uint8_t * key_data,
				 size_t key_len, void * user_data)
{
	Q_UNUSED(user_data);

	// 1 - name ("public" or "private" for own keys, name for contacts)
	// 2 - key blob
	// 3 - trusted (1 for true, 0 for false)
//	const QString save_stmt();
//	const QString del_stmt();
//	const QString &stmt = key_data?save_stmt:del_stmt;

	SQL_QUERY(key_data?"INSERT OR REPLACE INTO " IDENTITY_KEY_STORE_TABLE
					   " VALUES (?1, ?2, ?3)"
					  :"DELETE FROM " IDENTITY_KEY_STORE_TABLE
					   " WHERE " IDENTITY_KEY_STORE_NAME " IS ?1");

//	QSqlQuery query(stmt, db());

	query.bindValue(0, QString(addr_p->name));

	if (key_data) {
		query.bindValue(1, BYTE_ARRAY(key_data, key_len));
		query.bindValue(2, IDENTITY_KEY_TRUSTED);
	}

	if (!query.exec()) return -3;

	return 0;
}

int identityIsTrusted(const char * name, size_t name_len, uint8_t * key_data,
					  size_t key_len, void * user_data)
{
	Q_UNUSED(name_len);
	Q_UNUSED(user_data);

	signal_buffer * key_record = nullptr;

	SQL_QUERY("SELECT * FROM " IDENTITY_KEY_STORE_TABLE
			  " WHERE " IDENTITY_KEY_STORE_NAME " IS ?1");

	query.bindValue(0, QString(name));

	if (query.exec()) {
		if (query.next()) {
			// no entry = trusted, according to docs
			return 1;
		} else {
			// theoretically could be checked if trusted or not but it's TOFU
			QByteArray data = query.value(1).toByteArray();
			if (data.size() != int(key_len)) {
				qCritical("Key length does not match");
				return 0;
			}

			key_record = signal_buffer_create(DATA_SIZE(data));
			if (!key_record) {
				qCritical("Buffer could not be initialised");
				return -3;
			}

			if (memcmp(key_data, signal_buffer_data(key_record), key_len)) {
				qCritical("Key data does not match");
			}

			signal_buffer_bzero_free(key_record);
			return 1;
		}
	} else {
		qCritical("Failed executing SQL query");
		return -32;
	}
}

int identityAlwaysTrusted(const signal_protocol_address * addr_p, uint8_t * key_data,
						  size_t key_len, void * user_data)
{
  Q_UNUSED(addr_p);
  Q_UNUSED(key_data);
  Q_UNUSED(key_len);
  Q_UNUSED(user_data);

  return 1;
}

void identityDestroyCtx(void * user_data)
{
  Q_UNUSED(user_data);
  //const char stmt[] = "DELETE FROM identity_key_store; VACUUM;";

	//db_exec_quick(stmt, user_data);
}

int create()
{
	qDebug() << "OmemoStore::create()";

	const QStringList stmts(
		QStringList() << "CREATE TABLE IF NOT EXISTS " SESSION_STORE_TABLE "("
							 SESSION_STORE_NAME " TEXT NOT NULL, "
							 SESSION_STORE_DEVICE_ID " INTEGER NOT NULL, "
							 SESSION_STORE_RECORD " BLOB NOT NULL, "
						 "  PRIMARY KEY(" SESSION_STORE_NAME ", " SESSION_STORE_DEVICE_ID "))"
					  << "CREATE TABLE IF NOT EXISTS " PRE_KEY_STORE_TABLE "("
							 PRE_KEY_STORE_ID " INTEGER NOT NULL PRIMARY KEY, "
							 PRE_KEY_STORE_RECORD " BLOB NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " SIGNED_PRE_KEY_STORE_TABLE "("
							 SIGNED_PRE_KEY_STORE_ID " INTEGER NOT NULL PRIMARY KEY, "
							 SIGNED_PRE_KEY_STORE_RECORD " BLOB NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " IDENTITY_KEY_STORE_TABLE "("
							 IDENTITY_KEY_STORE_NAME " TEXT NOT NULL PRIMARY KEY, "
							 IDENTITY_KEY_STORE_KEY " BLOB NOT NULL, "
							 IDENTITY_KEY_STORE_TRUSTED " INTEGER NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " SETTINGS_STORE_TABLE "("
							 SETTINGS_STORE_NAME " TEXT NOT NULL PRIMARY KEY, "
							 SETTINGS_STORE_PROPERTY " INTEGER NOT NULL)");

	if (!db().transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	QSqlQuery query(db());
	for (QStringList::ConstIterator it = stmts.constBegin();
		 it != stmts.constEnd(); ++it) {
		query.prepare(*it);
		if (!query.exec()) {
			db().rollback();
			return -1;
		}
	}

	if (db().commit())
		return 0;
	else
		return -2;
}

/**
 * Drops all tables.
 *
 * @param axc_ctx_p Pointer to the axc context.
 */
int destroy() {
	const QStringList stmts(
		QStringList() << "DROP TABLE IF EXISTS " SESSION_STORE_TABLE
					  << "DROP TABLE IF EXISTS " PRE_KEY_STORE_TABLE
					  << "DROP TABLE IF EXISTS " SIGNED_PRE_KEY_STORE_TABLE
					  << "DROP TABLE IF EXISTS " IDENTITY_KEY_STORE_TABLE
					  << "DROP TABLE IF EXISTS " SETTINGS_STORE_TABLE);

	if (!db().transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	QSqlQuery query(db());
	for (QStringList::ConstIterator it = stmts.cbegin();
		 it != stmts.constEnd(); ++it) {
		query.prepare(*it);
		if (!query.exec()) {
			db().rollback();
			return -1;
		}
	}

	if (db().commit())
		return 0;
	else
		return -2;
}

int initStatusSet(const int status) {
	return propertySet(INIT_STATUS, status);
}

int initStatusGet(int *init_status_p)
{
	return propertyGet(INIT_STATUS, init_status_p);
}

int identitySetLocalRegistrationId(const uint32_t ARegistrationId)
{
	return propertySet(REG_ID, int(ARegistrationId)) ? -1 : 0;
}

int preKeyStoreList(signal_protocol_key_helper_pre_key_list_node *APreKeysHead)
{
	signal_buffer * key_buf_p = nullptr;
	signal_protocol_key_helper_pre_key_list_node * pre_keys_curr_p = nullptr;
	session_pre_key * pre_key_p = nullptr;

	if (!db().transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	SQL_QUERY("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE
			  " VALUES (?1, ?2)");

	pre_keys_curr_p = APreKeysHead;
	while (pre_keys_curr_p) {
		pre_key_p = signal_protocol_key_helper_key_list_element(pre_keys_curr_p);
		if (session_pre_key_serialize(&key_buf_p, pre_key_p)) {
			qCritical("failed to serialize pre key");
			db().rollback();
			return -1;
		}

		query.bindValue(0, session_pre_key_get_id(pre_key_p));
		query.bindValue(1, SBUF2BARR(key_buf_p));
		if (!query.exec()) {
			qCritical("Failed to execute query: \"%s\", error: %d (%s)",
					  query.lastQuery().toUtf8().data(),
					  query.lastError().number(),
					  query.lastError().text().toUtf8().data());
			db().rollback();
			return -3;
		}

		signal_buffer_bzero_free(key_buf_p);
		query.finish();

		pre_keys_curr_p = signal_protocol_key_helper_key_list_next(pre_keys_curr_p);
	}

	if (db().commit())
		return 0;
	else
		return -1;
}

int preKeyGetList(size_t AAmount, QMap<quint32, QByteArray> &APreKeys, signal_context *AContext)
{
	int rc = -1;
	char * errMsg = nullptr;
	session_pre_key * preKey = nullptr;
	signal_buffer * preKeyPublicSerialized = nullptr;
	signal_buffer * serializedKeypairData = nullptr;

	SQL_QUERY("SELECT * FROM " PRE_KEY_STORE_TABLE
			  " ORDER BY " PRE_KEY_STORE_ID " ASC LIMIT ?1");

	query.bindValue(0, AAmount);

	if (query.exec())
	{
		while (query.next()) {
			uint32_t id = query.value(0).toUInt();
			QByteArray data = query.value(1).toByteArray();
			serializedKeypairData = signal_buffer_create(DATA_SIZE(data));
			if (!serializedKeypairData) {
				errMsg = "failed to initialize buffer";
				rc = -3;
				goto cleanup;
			}

			rc = session_pre_key_deserialize(&preKey,
											 signal_buffer_data(serializedKeypairData),
											 signal_buffer_len(serializedKeypairData),
											 AContext);
			if (rc) {
				errMsg = "failed to deserialize keypair";
				goto cleanup;
			}

			ec_key_pair * preKeyPair = session_pre_key_get_key_pair(preKey);
			ec_public_key * preKeyPublic = ec_key_pair_get_public(preKeyPair);

			rc = ec_public_key_serialize(&preKeyPublicSerialized, preKeyPublic);
			if (rc) {
				errMsg = "failed to serialize public key";
				goto cleanup;
			}

			APreKeys.insert(id, SBUF2BARR(preKeyPublicSerialized));
			signal_buffer_free(preKeyPublicSerialized);
		}
	} else {
		errMsg = "sql error when retrieving keys";
		goto cleanup;
	}

	rc = 0;

cleanup:
	if (rc) {
		qCritical("%s: %d", errMsg, rc);
	}

	signal_buffer_free(serializedKeypairData);
	SIGNAL_UNREF(preKey);

	return rc;
}

int preKeyGetMaxId(uint32_t &AMaxId)
{
	char * err_msg = nullptr;
	int ret_val = 0;
	uint32_t id = 0;

	SQL_QUERY("SELECT MAX(" PRE_KEY_STORE_ID ") FROM " PRE_KEY_STORE_TABLE
			  " WHERE " PRE_KEY_STORE_ID " IS NOT ("
			  "   SELECT MAX(" PRE_KEY_STORE_ID ") FROM " PRE_KEY_STORE_TABLE
			  " )");

	if (query.exec()) {
		if (query.next()) {
			id = query.value(0).toUInt();
			if (!id) {
				err_msg = "db not initialized";
				ret_val = -1;
			} else {
				AMaxId = id;
				ret_val = 0;
			}
		} else {
			err_msg = "No data in database";
			ret_val = -2;
		}
	} else {
		err_msg = "SQL query execution failed";
		ret_val = query.lastError().number();
	}

	if (ret_val)
		qCritical("%s: %d", err_msg, ret_val);

	return ret_val;
}

int preKeyGetCount()
{
	int ret_val = 0;
	char * err_msg = nullptr;

	SQL_QUERY("SELECT count(" PRE_KEY_STORE_ID") FROM " PRE_KEY_STORE_TABLE);

	if (query.exec()) {
		if (query.next()) {
			ret_val = query.value(0).toInt();
		} else {
			err_msg = "SQL query returned empty result";
			ret_val = -2;
		}
	} else {
		err_msg = "SQL query execution failed";
		ret_val = -1;
	}

	if (ret_val<0)
		qCritical("%s: %d", err_msg, ret_val);

	return ret_val;
}

}
