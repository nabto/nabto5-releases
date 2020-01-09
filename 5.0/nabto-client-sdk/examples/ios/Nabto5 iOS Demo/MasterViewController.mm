//
//  MasterViewController.m
//  Nabto5 iOS Demo
//
//  Created by Ulrik Gammelby on 08/05/2019.
//  Copyright © 2019 Nabto ApS. All rights reserved.
//

#import <nabto/nabto_client.h>
#import "MasterViewController.h"
#import "DetailViewController.h"

@interface MasterViewController ()

@property NSMutableArray *objects;
@end

@implementation MasterViewController

struct streamContext {
    NabtoClient* client;
    NabtoClientConnection* connection;
    NabtoClientStream* stream;
    size_t lastWritten;        // how many bytes written by last stream write operation
    size_t lastRead;           // how many bytes actually read
    CFAbsoluteTime lastWriteTime;
    uint8_t buffer[4];
    size_t count;
    void* self;
};

static void cleanup(struct streamContext* ctx) {
    if (ctx->client && ctx->connection) {
        NabtoClientFuture* future = nabto_client_future_new(ctx->client);
        nabto_client_connection_close(ctx->connection, future);
        nabto_client_future_wait(future);
        nabto_client_future_free(future);
        nabto_client_connection_free(ctx->connection);
        ctx->connection = NULL;
        nabto_client_free(ctx->client);
        ctx->client = NULL;
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    self.navigationItem.leftBarButtonItem = self.editButtonItem;

    UIBarButtonItem *addButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(handleAdd:)];
    self.navigationItem.rightBarButtonItem = addButton;
    UISplitViewController* controller = self.splitViewController;
    self.detailViewController = (DetailViewController *)[[controller.viewControllers lastObject] topViewController];
}


- (void)viewWillAppear:(BOOL)animated {
    self.clearsSelectionOnViewWillAppear = self.splitViewController.isCollapsed;
    [super viewWillAppear:animated];
}

static void connectedCallback(NabtoClientFuture *future, NabtoClientError ec, void *data)
{
    struct streamContext* ctx = (struct streamContext*)data;
    MasterViewController* self = (__bridge MasterViewController*)ctx->self;
    [self insertNewObject:[self stringWithNabtoError:@"Connect completed"
                                               error:ec]];
    if (ec == NABTO_CLIENT_EC_OK) {
        [self nabtoDemoStreamOpen:ctx];
    } else {
        cleanup(ctx);
    }
}

static void streamOpenedCallback(NabtoClientFuture* future, NabtoClientError ec, void* data)
{
    struct streamContext* ctx = (struct streamContext*)data;
    MasterViewController* self = (__bridge MasterViewController*)ctx->self;
    [self insertNewObject:[self stringWithNabtoError:@"Stream open completed"
                                               error:ec]];
    if (ec == NABTO_CLIENT_EC_OK) {
        [self nabtoDemoStreamWrite:ctx];
    } else {
        cleanup(ctx);
    }
}

static void streamWriteDoneCallback(NabtoClientFuture* future, NabtoClientError ec, void* data)
{
    NSLog(@"streamWriteDoneCallback");
    struct streamContext* ctx = (struct streamContext*)data;
    MasterViewController* self = (__bridge MasterViewController*)ctx->self;
    if (ec == NABTO_CLIENT_EC_OK) {
        [self insertNewObject:[NSString stringWithFormat:@"Wrote %lu bytes", ctx->lastWritten]];
        [self nabtoDemoStreamRead:ctx];
    } else {
        [self insertNewObject:[self stringWithNabtoError:@"Stream write failed"
                                                   error:ec]];
        cleanup(ctx);
    }
}

static void streamReadDoneCallback(NabtoClientFuture* future, NabtoClientError ec, void* data)
{
    struct streamContext* ctx = (struct streamContext*)data;
    MasterViewController* self = (__bridge MasterViewController*)ctx->self;
    if (ec == NABTO_CLIENT_EC_OK) {
        CFAbsoluteTime latencyMillis = 1000 * (CFAbsoluteTimeGetCurrent() - ctx->lastWriteTime);
        [self insertNewObject:[NSString stringWithFormat:@"Read %lu bytes back in %.2f ms", ctx->lastRead, latencyMillis]];
        ctx->count++;
        if (ctx->count < 100) {
            [self nabtoDemoStreamWrite:ctx];
            NabtoClientConnectionType type;
            nabto_client_connection_get_type(ctx->connection, &type);
            [self insertNewObject:[NSString stringWithFormat:@"type: %d", type]];
        } else {
            [self insertNewObject:[NSString stringWithFormat:@"Done, cleaning up"]];
            cleanup(ctx);
        }
    } else {
        [self insertNewObject:[self stringWithNabtoError:@"Stream write failed"
                                                   error:ec]];
        cleanup(ctx);
    }
}

- (void)nabtoDemoStreamOpen:(struct streamContext*)ctx {
    ctx->stream = nabto_client_stream_new(ctx->connection);
    NabtoClientFuture* future = nabto_client_future_new(ctx->client);
    nabto_client_stream_open(ctx->stream, future, 42);
    nabto_client_future_set_callback(future, streamOpenedCallback, ctx);
}

- (void)nabtoDemoStreamWrite:(struct streamContext*)ctx {
    const char* data = "ping";
    ctx->lastWritten = strlen(data);
    ctx->lastWriteTime = CFAbsoluteTimeGetCurrent();
    NabtoClientFuture* future = nabto_client_future_new(ctx->client);
    nabto_client_stream_write(ctx->stream, future, data, strlen(data));
    nabto_client_future_set_callback(future, streamWriteDoneCallback, ctx);
}

- (void)nabtoDemoStreamRead:(struct streamContext*)ctx {
    NabtoClientFuture* future = nabto_client_future_new(ctx->client);
    nabto_client_stream_read_all(ctx->stream, future, ctx->buffer, sizeof(ctx->buffer), &ctx->lastRead);
    nabto_client_future_set_callback(future, streamReadDoneCallback, ctx);
}

- (void)nabtoDemoConnect {
    // TODO... disable connect button until complete or manage multiple concurrent contexts/connections
    const char* clientPrivateKey =
            "-----BEGIN EC PARAMETERS-----\r\n"
            "BggqhkjOPQMBBw==\r\n"
            "-----END EC PARAMETERS-----\r\n"
            "-----BEGIN EC PRIVATE KEY-----\r\n"
            "MHcCAQEEIBnZr32pwf7eH5vLqDD5hgzR3EzoEJVZ0tT4QqjakFrGoAoGCCqGSM49\r\n"
            "AwEHoUQDQgAEPexGIS7sjA6BmOKbvCsu3/I/qxjY2CTE5RANbiaw7xWwHEcexYYR\r\n"
            "nM7sgVTdDTc2zrOYpqAA0a2k3UnUJloxFg==\r\n"
            "-----END EC PRIVATE KEY-----\r\n";

    const char* serverUrl = "https://pr-agkywx3n.clients.dev.nabto.net";
    const char* productId = "pr-agkywx3n";
    const char* deviceId = "de-eztprfqh";
    const char* serverKey = "sk-55b771cc30d531201837b5e49ee1bcd6";

    [self insertNewObject:[NSString stringWithFormat:@"Connecting to %s::%s...", productId, deviceId]];

    struct streamContext* ctx = (struct streamContext*)malloc(sizeof(struct streamContext));
    memset(ctx, 0, sizeof(struct streamContext));

    ctx->self =  (__bridge void *) self;
    ctx->client = nabto_client_new();
    ctx->connection = nabto_client_connection_new(ctx->client);
    NabtoClientError ec;

    [self insertNewObject:[NSString stringWithFormat:@"Nabto version: %s", nabto_client_version()]];

    if ((ec = nabto_client_connection_set_server_url(ctx->connection, serverUrl)) != NABTO_CLIENT_EC_OK) {
        [self insertNewObject:[self stringWithNabtoError:@"Error" error:ec]];
    }

    if ((ec = nabto_client_connection_set_product_id(ctx->connection, productId)) != NABTO_CLIENT_EC_OK) {
        [self insertNewObject:[self stringWithNabtoError:@"Error" error:ec]];
    }

    if ((ec = nabto_client_connection_set_device_id(ctx->connection, deviceId)) != NABTO_CLIENT_EC_OK) {
        [self insertNewObject:[self stringWithNabtoError:@"Error" error:ec]];
    }

    if ((ec = nabto_client_connection_set_private_key(ctx->connection, clientPrivateKey)) != NABTO_CLIENT_EC_OK) {
        [self insertNewObject:[self stringWithNabtoError:@"Error" error:ec]];
    }

    if ((ec = nabto_client_connection_set_server_api_key(ctx->connection, serverKey)) != NABTO_CLIENT_EC_OK) {
        [self insertNewObject:[self stringWithNabtoError:@"Error" error:ec]];
    }

    NabtoClientFuture* future = nabto_client_future_new(ctx->client);
//    char* info;
//    nabto_client_connection_get_info(ctx->connection, &info);
//    [self insertNewObject:[NSString stringWithFormat:@"Connect info: %s", info]];
//    nabto_client_string_free(info);
    nabto_client_connection_connect(ctx->connection, future);
    nabto_client_future_set_callback(future, connectedCallback, ctx);
}

- (NSString *)stringWithNabtoError:(NSString*)message error:(NabtoClientError)ec {
    const char* status = ec == NABTO_CLIENT_EC_OK ? "OK" : nabto_client_error_get_message(ec);
    return [NSString stringWithFormat:@"%@ - Status %d: %s", message, ec, status];
}

- (void)handleAdd:(id)sender {
    [self nabtoDemoConnect];
}

- (void)insertNewObject:(NSObject *)obj {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!self.objects) {
            self.objects = [[NSMutableArray alloc] init];
        }
        [self.objects insertObject:obj atIndex:0];
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:0 inSection:0];
        NSLog(@"insertNewObject: %@", [obj description]);
        [self.tableView insertRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    });
}


#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"showDetail"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        NSObject *object = self.objects[indexPath.row];
        DetailViewController *controller = (DetailViewController *)[[segue destinationViewController] topViewController];
        [controller setDetailItem:object];
        controller.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
        controller.navigationItem.leftItemsSupplementBackButton = YES;
    }
}


#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.objects.count;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"Cell" forIndexPath:indexPath];

    NSObject *object = self.objects[indexPath.row];
    cell.textLabel.text = [object description];
    return cell;
}


- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}


- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        [self.objects removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }
}


@end
